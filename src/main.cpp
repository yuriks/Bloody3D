#include "Heatwave.hpp"
#include "Engine.hpp"
#include "gl/VertexArrayObject.hpp"
#include "gl/BufferObject.hpp"
#include "gl/Shader.hpp"
#include "gl/ShaderProgram.hpp"
#include "gl/Framebuffer.hpp"
#include "math/mat.hpp"
#include "math/MatrixTransform.hpp"
#include "math/Quaternion.hpp"
#include "math/misc.hpp"
#include "mesh/Material.hpp"
#include "mesh/VertexFormats.hpp"
#include "mesh/GPUMesh.hpp"
#include "mesh/ObjLoader.hpp"
#include "texture/Texture.hpp"
#include "mesh/HWMesh.hpp"
#include "scene/Scene.hpp"
#include "scene/RenderContext.hpp"
#include "scene/PostProcessing.hpp"
#include "scene/DirectionalLight.hpp"
#include "scene/OmniLight.hpp"
#include "scene/SpotLight.hpp"
#include "editor/AssetProcessing.hpp"
#include "util/mmap.hpp"
#include "util/StringHash.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>

#include <GL/gl3w.h>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static GLFWwindow* glfw_window = nullptr;

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	if ((type != GL_DEBUG_TYPE_PERFORMANCE_ARB && type != GL_DEBUG_TYPE_OTHER_ARB) || severity != GL_DEBUG_SEVERITY_LOW_ARB)
		std::cerr << message << std::endl;
	if ((type != GL_DEBUG_TYPE_PERFORMANCE_ARB && type != GL_DEBUG_TYPE_OTHER_ARB) || severity == GL_DEBUG_SEVERITY_HIGH_ARB)
		DebugBreak(); // Breakpoint
}

bool init_window()
{
	if (!glfwInit())
	{
		char tmp;
		std::cerr << "Failed to initialize GLFW." << std::endl;
		std::cin >> tmp;
		return false;
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfw_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Bloody3D", nullptr, nullptr);
	if (glfw_window == nullptr)
	{
		char tmp;
		std::cerr << "Failed to open window." << std::endl;
		std::cin >> tmp;
		return false;
	}

	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(1);

	if (gl3wInit() != 0) {
		char tmp;
		std::cerr << "Failed to initialize gl3w." << std::endl;
		std::cin >> tmp;
		return false;
	} else if (!gl3wIsSupported(3, 3)) {
		char tmp;
		std::cerr << "OpenGL 3.3 not supported." << std::endl;
		std::cin >> tmp;
		return false;
	}

	if (glDebugMessageCallbackARB) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
		glDebugMessageCallbackARB(debug_callback, 0);
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void setupTestScene(Engine& engine, scene::Scene& scene) {
	Handle mesh_id;
	{
		Handle mat_id;
		{
			MaterialTemplate material_template;
			material_template.attachShaders("test");
			material_template.options_size = 0;

			mat_id = engine.materials.insert(material_template.compile());
			scene.named_handles.insert(std::make_pair("standard_material", mat_id));
		}
	}
}

void renderCamera(
	const Engine& engine, const scene::Scene& scene, const scene::Camera& camera,
	scene::GBufferSet& def_buffers, scene::ShadingBufferSet& shading_buffers,
	const math::mat4* model2world_mats, const math::mat4* model2world_inv_mats)
{
	using namespace scene;

	scene::SystemUniformBlock sys_uniforms;
	sys_uniforms.projection_mat = math::mat_transform::perspective_proj(camera.fov, engine.render_context.aspect_ratio, camera.clip_near, camera.clip_far);
	math::mat4 world2view_mat = model2world_inv_mats[scene.transforms.getPoolIndex(camera.transform)];

	scene::renderGeometry(scene, world2view_mat, model2world_mats, def_buffers, engine.render_context, sys_uniforms);

	shading_buffers.fbo.bind(GL_DRAW_FRAMEBUFFER);
	bindGBufferTextures(def_buffers);
	math::vec4 clear_color = {0, 0, 0, 0};
	glClearBufferfv(GL_COLOR, 0, clear_color.data);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	{
		scene::ShadeLightSetParams p = {
			&world2view_mat, model2world_mats, &engine, &scene, &sys_uniforms
		};

		scene::shadeLightSet<DirectionalLight, GPUDirectionalLight>(
			scene.lights_dir.pool, engine.dirlight, p);
		scene::shadeLightSet<OmniLight, GPUOmniLight>(
			scene.lights_omni.pool, engine.omnilight, p);
		scene::shadeLightSet<SpotLight, GPUSpotLight>(
			scene.lights_spot.pool, engine.spotlight, p);
	}

	for (int t = 0; t < 3; ++t) {
		glActiveTexture(GL_TEXTURE0 + t);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void renderScene(
	const Engine& engine, const scene::Scene& scene,
	scene::GBufferSet& def_buffers, scene::ShadingBufferSet& shading_buffers)
{
	using namespace scene;

	std::vector<math::mat4> model2world_mats(scene.transforms.pool.size());
	std::vector<math::mat4> model2world_inv_mats(scene.transforms.pool.size());
	scene::calculateModel2WorldMatrices(scene.transforms,
		model2world_mats.data(), model2world_inv_mats.data());

	renderCamera(engine, scene, *scene.cameras[scene.active_camera],
		def_buffers, shading_buffers, model2world_mats.data(), model2world_inv_mats.data());

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	engine.null_vao.bind();
	scene::tonemap(shading_buffers, *engine.materials[engine.tonemap_material], engine.render_context);
	glBindVertexArray(0);
}

void testParse(scene::Scene& scene);

int main(int argc, char *argv[])
{
	if (argc > 1 && std::strcmp(argv[1], "-a") == 0) {
		return editor::asset_processing(argc - 2, argv + 2);
	}

	if (!init_window())
		return 1;

	{
		Engine engine;
		engine.render_context.setScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		loadEngineMaterials(engine);

		bool running = true;

		glEnable(GL_CULL_FACE);
		//glEnable(GL_DEPTH_CLAMP);
		glFrontFace(GL_CW);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		scene::GBufferSet def_buffers;
		def_buffers.initialize(engine.render_context.screen_width, engine.render_context.screen_height);
		scene::ShadingBufferSet shading_buffers;
		shading_buffers.initialize(engine.render_context.screen_width, engine.render_context.screen_height, def_buffers.depth_tex);

		scene::Scene scene(&engine);
		setupTestScene(engine, scene);
		testParse(scene);

		Handle wall_t = scene.named_handles.at("parent_mesh");
		scene.active_camera = scene.cameras.makeHandle(0);

		double elapsed_game_time = 0.;
		double elapsed_real_time = 0.;
		double last_frame_time;

		double last_mouse_pos[2];
		glfwGetCursorPos(glfw_window, &last_mouse_pos[0], &last_mouse_pos[1]);

		while (running)
		{
			double frame_start = glfwGetTime();

			int i;
			for (i = 0; elapsed_game_time < elapsed_real_time && i < 5; ++i)
			{
				double cur_mouse_pos[2];
				glfwGetCursorPos(glfw_window, &cur_mouse_pos[0], &cur_mouse_pos[1]);
				float x_mdelta = float(cur_mouse_pos[0] - last_mouse_pos[0]);
				float y_mdelta = float(cur_mouse_pos[1] - last_mouse_pos[1]);

				static const float ROT_SPEED = 0.02f;
				static const float MOUSE_ROT_SPEED = 0.01f;

				math::Quaternion& rot_amount = scene.transforms[wall_t]->rot;

				if (glfwGetKey(glfw_window, 'A')) rot_amount = math::Quaternion(math::up, ROT_SPEED) * rot_amount;
				if (glfwGetKey(glfw_window, 'D')) rot_amount = math::Quaternion(math::up, -ROT_SPEED) * rot_amount;

				if (glfwGetKey(glfw_window, 'W')) rot_amount = math::Quaternion(math::right, ROT_SPEED) * rot_amount;
				if (glfwGetKey(glfw_window, 'S')) rot_amount = math::Quaternion(math::right, -ROT_SPEED) * rot_amount;

				if (glfwGetKey(glfw_window, 'Q')) rot_amount = math::Quaternion(math::forward, ROT_SPEED) * rot_amount;
				if (glfwGetKey(glfw_window, 'E')) rot_amount = math::Quaternion(math::forward, -ROT_SPEED) * rot_amount;

				rot_amount = math::Quaternion(math::up, -x_mdelta * MOUSE_ROT_SPEED) * rot_amount;
				rot_amount = math::Quaternion(math::right, -y_mdelta * MOUSE_ROT_SPEED) * rot_amount;

				last_mouse_pos[0] = cur_mouse_pos[0];
				last_mouse_pos[1] = cur_mouse_pos[1];

				elapsed_game_time += 1./60.;
			}

			renderScene(engine, scene, def_buffers, shading_buffers);

			glfwSwapBuffers(glfw_window);
			glfwPollEvents();

			double tmp = elapsed_real_time + (glfwGetTime() - frame_start);
			if (elapsed_game_time > tmp) {
				const std::chrono::duration<double> sleep_duration(elapsed_game_time - tmp - 0.01);
				std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::microseconds>(sleep_duration));
			}

			last_frame_time = glfwGetTime() - frame_start;
			elapsed_real_time += last_frame_time;
			if (i == 5)
				elapsed_game_time = elapsed_real_time;

			running = glfwWindowShouldClose(glfw_window) == 0 && glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) != GLFW_PRESS;
		}
	}

	glfwDestroyWindow(glfw_window);
	glfwTerminate();

	return 0;
}
