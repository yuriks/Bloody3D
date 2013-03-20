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

#include <GL/gl3w.h>

//#define GLFW_GL3_H
#include <GL/glfw.h>

static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	if ((type != GL_DEBUG_TYPE_PERFORMANCE_ARB && type != GL_DEBUG_TYPE_OTHER_ARB) || severity != GL_DEBUG_SEVERITY_LOW_ARB)
		std::cerr << message << std::endl;
	if ((type != GL_DEBUG_TYPE_PERFORMANCE_ARB && type != GL_DEBUG_TYPE_OTHER_ARB) || severity == GL_DEBUG_SEVERITY_HIGH_ARB)
		DebugBreak(); // Breakpoint
}

bool init_window()
{
	if (glfwInit() != GL_TRUE)
	{
		char tmp;
		std::cerr << "Failed to initialize GLFW." << std::endl;
		std::cin >> tmp;
		return false;
	}

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (glfwOpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 8, 8, 8, 0, 0, 0, GLFW_WINDOW) != GL_TRUE)
	{
		char tmp;
		std::cerr << "Failed to open window." << std::endl;
		std::cin >> tmp;
		return false;
	}

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

	glfwDisable(GLFW_MOUSE_CURSOR);

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

		GPUMesh mesh;

		util::MMapHandle mmap_h = util::mmapFile("data/panel_beams.hwmesh");
		assert(mmap_h != -1);

		mesh::loadHWMesh(util::mmapGetData(mmap_h), util::fnv_hash("panel_beams"), mesh);

		util::mmapClose(mmap_h);

		mesh.material_id = mat_id;

		{
			TextureTemplate tex_template;
			tex_template.file = "panel_beams_diffuse.png";
			tex_template.flags = TEXF_SRGB;
			scene.named_handles.insert(
				std::make_pair("tex_panel_beams_diffuse", engine.textures.insert(tex_template.compile())));

			tex_template.file = "panel_beams_normal.png";
			tex_template.flags = TEXF_NONE;
			scene.named_handles.insert(
				std::make_pair("tex_panel_beams_normal", engine.textures.insert(tex_template.compile())));
		}

		mesh_id = engine.gpu_meshes.insert(std::move(mesh));
		scene.named_handles.insert(std::make_pair("panel_beams", mesh_id));
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

		int last_mouse_pos[2];
		glfwGetMousePos(&last_mouse_pos[0], &last_mouse_pos[1]);

		while (running)
		{
			double frame_start = glfwGetTime();

			int i;
			for (i = 0; elapsed_game_time < elapsed_real_time && i < 5; ++i)
			{
				int cur_mouse_pos[2];
				glfwGetMousePos(&cur_mouse_pos[0], &cur_mouse_pos[1]);
				float x_mdelta = float(cur_mouse_pos[0] - last_mouse_pos[0]);
				float y_mdelta = float(cur_mouse_pos[1] - last_mouse_pos[1]);

				static const float ROT_SPEED = 0.02f;
				static const float MOUSE_ROT_SPEED = 0.01f;

				math::Quaternion& rot_amount = scene.transforms[wall_t]->rot;

				if (glfwGetKey('A')) rot_amount = math::Quaternion(math::up, ROT_SPEED) * rot_amount;
				if (glfwGetKey('D')) rot_amount = math::Quaternion(math::up, -ROT_SPEED) * rot_amount;

				if (glfwGetKey('W')) rot_amount = math::Quaternion(math::right, ROT_SPEED) * rot_amount;
				if (glfwGetKey('S')) rot_amount = math::Quaternion(math::right, -ROT_SPEED) * rot_amount;

				if (glfwGetKey('Q')) rot_amount = math::Quaternion(math::forward, ROT_SPEED) * rot_amount;
				if (glfwGetKey('E')) rot_amount = math::Quaternion(math::forward, -ROT_SPEED) * rot_amount;

				rot_amount = math::Quaternion(math::up, -x_mdelta * MOUSE_ROT_SPEED) * rot_amount;
				rot_amount = math::Quaternion(math::right, -y_mdelta * MOUSE_ROT_SPEED) * rot_amount;

				last_mouse_pos[0] = cur_mouse_pos[0];
				last_mouse_pos[1] = cur_mouse_pos[1];

				elapsed_game_time += 1./60.;
			}

			renderScene(engine, scene, def_buffers, shading_buffers);

			glfwSwapBuffers();

			double tmp = elapsed_real_time + (glfwGetTime() - frame_start);
			if (elapsed_game_time > tmp)
				glfwSleep(elapsed_game_time - tmp - 0.01);

			last_frame_time = glfwGetTime() - frame_start;
			elapsed_real_time += last_frame_time;
			if (i == 5)
				elapsed_game_time = elapsed_real_time;

			running = glfwGetWindowParam(GLFW_OPENED) != 0 && glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS;
		}
	}

	glfwTerminate();

	return 0;
}
