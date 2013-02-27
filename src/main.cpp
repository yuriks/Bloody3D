#include "Heatwave.hpp"
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
#include "mesh/TextureManager.hpp"
#include "mesh/HWMesh.hpp"
#include "scene/Scene.hpp"
#include "scene/RenderContext.hpp"
#include "scene/PostProcessing.hpp"
#include "editor/AssetProcessing.hpp"
#include "util/mmap.hpp"
#include "util/StringHash.hpp"

#include <iostream>
#include <fstream>

#include "GL3/gl3w.h"

//#define GLFW_GL3_H
#include <GL/glfw.h>

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
	if (glfwOpenWindow(800, 600, 8, 8, 8, 0, 0, 0, GLFW_WINDOW) != GL_TRUE)
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

	glViewport(0, 0, 800, 600);

	glfwDisable(GLFW_MOUSE_CURSOR);

	return true;
}

struct MatUniforms : public MaterialUniforms {
	float dummy;
};

struct ShadingUniforms : public MaterialUniforms {
	float dummy;
};

int main(int argc, char *argv[])
{
	if (argc > 1 && std::strcmp(argv[1], "-a") == 0) {
		return editor::asset_processing(argc - 2, argv + 2);
	}

	if (!init_window())
		return 1;

	{
		scene::RenderContext render_context(800, 600);
		scene::Scene scene;

		int mesh_id;
		{
			int mat_id;
			{
				Material material;
				material.loadFromFiles("test.vert", "test.frag");
				material.setOptionsSize(sizeof(MatUniforms));

				mat_id = scene.addMaterial(std::move(material));
			}

			GPUMesh mesh;

			util::MMapHandle mmap_h = util::mmapFile("data/panel_beams.hwmesh");
			assert(mmap_h != -1);

			mesh::loadHWMesh(util::mmapGetData(mmap_h), util::fnv_hash("panel_beams"), mesh);

			util::mmapClose(mmap_h);

			mesh.material_id = mat_id;

			{
				auto u = std::make_shared<MatUniforms>();
				u->dummy = 1.f;

				MaterialOptions mtl_options;
				mtl_options.uniforms = std::move(u);
				mtl_options.texture_ids.fill(-1);
				mtl_options.texture_ids[0] = scene.tex_manager.loadTexture("panel_beams_diffuse.png", TEXF_SRGB);
				mtl_options.texture_ids[1] = scene.tex_manager.loadTexture("panel_beams_normal.png");
				mesh.material_options = mtl_options;
			}

			mesh_id = scene.addMesh(std::move(mesh));
		}

		scene::MeshInstanceHandle insth = scene.newInstance(mesh_id);
		{
			scene::MeshInstance& inst = insth.resolve(scene);
			inst.pos_scale = math::mvec4(0.f, 0.f, 0.f, 1.f);
			inst.rot = math::Quaternion();
		}

		std::vector<scene::DirectionalLight> directional_lights;
		{
			scene::DirectionalLight light;
			light.direction = math::normalized(math::mvec3(-0.5f, -1.f, 0.5f));
			light.color = 2.5f * math::vec3_1;
			directional_lights.push_back(light);
		}

		scene::Camera camera;
		camera.fov = 45.f;
		camera.clip_near = 0.1f;
		camera.clip_far = 500.f;
		camera.pos = math::mvec3(0.f, 0.f, -5.f);
		camera.rot = math::Quaternion();

		{
			Material dirlight_material;
			dirlight_material.loadFromFiles("fullscreen_triangle.vert", "light_directional.frag");
			dirlight_material.setOptionsSize(sizeof(scene::DirectionalLight));
			Material tonemap_material;
			tonemap_material.loadFromFiles("fullscreen_triangle.vert", "tonemap.frag");
			tonemap_material.setOptionsSize(sizeof(ShadingUniforms));

			bool running = true;

			glClearColor(0.0f, 0.0f, 0.0f, 1.f);
			glEnable(GL_CULL_FACE);
			//glEnable(GL_DEPTH_CLAMP);
			glFrontFace(GL_CW);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			scene::GBufferSet def_buffers;
			def_buffers.initialize(render_context.screen_width, render_context.screen_height);
			scene::ShadingBufferSet shading_buffers;
			shading_buffers.initialize(render_context.screen_width, render_context.screen_height, def_buffers.depth_tex);

			double elapsed_game_time = 0.;
			double elapsed_real_time = 0.;
			double last_frame_time;

			math::Quaternion rot_amount(math::up, math::pi);

			int last_mouse_pos[2];
			glfwGetMousePos(&last_mouse_pos[0], &last_mouse_pos[1]);

			while (running)
			{
				double frame_start = glfwGetTime();

				int i;
				for (i = 0; elapsed_game_time < elapsed_real_time && i < 5; ++i)
				{
					static const float ROT_SPEED = 0.02f;
					static const float MOUSE_ROT_SPEED = 0.01f;

					if (glfwGetKey('A')) rot_amount = math::Quaternion(math::up, ROT_SPEED) * rot_amount;
					if (glfwGetKey('D')) rot_amount = math::Quaternion(math::up, -ROT_SPEED) * rot_amount;

					if (glfwGetKey('W')) rot_amount = math::Quaternion(math::right, ROT_SPEED) * rot_amount;
					if (glfwGetKey('S')) rot_amount = math::Quaternion(math::right, -ROT_SPEED) * rot_amount;

					if (glfwGetKey('Q')) rot_amount = math::Quaternion(math::forward, ROT_SPEED) * rot_amount;
					if (glfwGetKey('E')) rot_amount = math::Quaternion(math::forward, -ROT_SPEED) * rot_amount;

					int cur_mouse_pos[2];
					glfwGetMousePos(&cur_mouse_pos[0], &cur_mouse_pos[1]);

					float x_mdelta = float(cur_mouse_pos[0] - last_mouse_pos[0]);
					float y_mdelta = float(cur_mouse_pos[1] - last_mouse_pos[1]);
					rot_amount = math::Quaternion(math::up, -x_mdelta * MOUSE_ROT_SPEED) * rot_amount;
					rot_amount = math::Quaternion(math::right, -y_mdelta * MOUSE_ROT_SPEED) * rot_amount;

					last_mouse_pos[0] = cur_mouse_pos[0];
					last_mouse_pos[1] = cur_mouse_pos[1];

					{
						// TODO: Experiment with changing to immediate mode style API
						scene::MeshInstance& inst = insth.resolve(scene);
						inst.rot = rot_amount;
					}

					elapsed_game_time += 1./60.;
				}

				scene::SystemUniformBlock sys_uniforms;
				sys_uniforms.projection_mat = math::mat_transform::perspective_proj(camera.fov, render_context.aspect_ratio, camera.clip_near, camera.clip_far);

				scene::renderGeometry(scene, camera, def_buffers, render_context, sys_uniforms);

				shading_buffers.fbo.bind(GL_DRAW_FRAMEBUFFER);
				bindGBufferTextures(def_buffers);
				glClear(GL_COLOR_BUFFER_BIT);
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
				scene::shadeDirectionalLights(directional_lights, dirlight_material, render_context, sys_uniforms);
				for (int t = 0; t < 3; ++t) {
					glActiveTexture(GL_TEXTURE0 + t);
					glBindTexture(GL_TEXTURE_2D, 0);
				}

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				scene::tonemap(shading_buffers, tonemap_material, render_context);

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
	}

	glfwTerminate();

	return 0;
}
