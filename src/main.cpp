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
#include "texture/TextureManager.hpp"
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

int main(int argc, char *argv[])
{
	if (argc > 1 && std::strcmp(argv[1], "-a") == 0) {
		return editor::asset_processing(argc - 2, argv + 2);
	}

	if (!init_window())
		return 1;

	{
		Engine engine;

		scene::RenderContext render_context(WINDOW_WIDTH, WINDOW_HEIGHT);
		scene::Scene scene(&engine);

		Handle mesh_id;
		{
			Handle mat_id;
			{
				MaterialTemplate material_template;
				material_template.attachShaders("test");
				material_template.options_size = 0;

				mat_id = engine.materials.insert(material_template.compile());
			}

			GPUMesh mesh;

			util::MMapHandle mmap_h = util::mmapFile("data/panel_beams.hwmesh");
			assert(mmap_h != -1);

			mesh::loadHWMesh(util::mmapGetData(mmap_h), util::fnv_hash("panel_beams"), mesh);

			util::mmapClose(mmap_h);

			mesh.material_id = mat_id;

			{
				MaterialOptions mtl_options;
				mtl_options.uniforms = nullptr;
				mtl_options.texture_ids[0] = engine.texture_manager.getTexture("panel_beams_diffuse.png", TEXF_SRGB);
				mtl_options.texture_ids[1] = engine.texture_manager.getTexture("panel_beams_normal.png");
				mesh.material_options = mtl_options;
			}

			mesh_id = engine.gpu_meshes.insert(std::move(mesh));
		}

		scene::Transform wall_tt;
		wall_tt.rot = math::Quaternion(math::up, math::pi);
		Handle wall_t_h = scene.transforms.insert(wall_tt);
		Handle wall_meshinst_h = scene.mesh_instances.insert(scene::MeshInstance(wall_t_h, mesh_id));

		{
			scene::DirectionalLight light;
			scene::Transform t;

			t.rot = math::shortestArc(math::vec3_z, math::normalized(math::mvec3(-0.5f, -1.f, 0.5f)));
			light.color = 2.5f * math::vec3_1;
			light.transform = scene.transforms.insert(t);
			scene.lights_dir.insert(light);

			t.rot = math::shortestArc(math::vec3_z, math::vec3_y);
			light.color = math::mvec3(0.1f, 0.1f, 0.8f);
			light.transform = scene.transforms.insert(t);
			scene.lights_dir.insert(light);
		}

		{
			scene::OmniLight light;
			scene::Transform t;

			t.pos = math::mvec3(3.0f, 3.0f, 0.0f);
			light.color = math::mvec3(32.0f, 0.0f, 0.0f);
			light.transform = scene.transforms.insert(t);
			scene.lights_omni.insert(light);
		}

		{
			scene::SpotLight light;
			scene::Transform t;

			t.pos = math::mvec3(0.0f, 0.0f, 3.0f);
			t.rot = math::Quaternion(math::vec3_y, math::pi);
			t.parent = wall_t_h;
			light.exponent = 64;
			light.color = math::mvec3(0.0f, 32.0f, 0.0f);
			light.transform = scene.transforms.insert(t);
			scene.lights_spot.insert(light);
		}

		scene::Camera camera;
		camera.fov = 45.f;
		camera.clip_near = 0.1f;
		camera.clip_far = 500.f;

		{
			scene::Transform t;
			t.pos = math::mvec3(0.f, 0.f, -3.5f);
			camera.transform = scene.transforms.insert(t);
		}

		{
			MaterialTemplate material_template;
			material_template.attachShaders("light_directional");
			material_template.options_size = 0;
			Material dirlight_material = material_template.compile();

			gl::VertexArrayObject dirlight_vao;
			gl::BufferObject dirlight_vbo;

			// Setup Directional Light vertex attribs
			{
				using scene::GPUDirectionalLight;
				dirlight_vao.bind();

				dirlight_vbo.bind(GL_ARRAY_BUFFER);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
					sizeof(GPUDirectionalLight), (void*)offsetof(GPUDirectionalLight, direction));
				glEnableVertexAttribArray(0);
				glVertexAttribDivisor(0, 1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE,
					sizeof(GPUDirectionalLight), (void*)offsetof(GPUDirectionalLight, color));
				glEnableVertexAttribArray(1);
				glVertexAttribDivisor(1, 1);
			}

			material_template.clear();
			material_template.attachShaders("light_omni");
			material_template.options_size = 0;
			Material omnilight_material = material_template.compile();

			gl::VertexArrayObject omnilight_vao;
			gl::BufferObject omnilight_vbo;

			// Setup Omnilight vertex attribs
			{
				using scene::GPUOmniLight;
				omnilight_vao.bind();

				omnilight_vbo.bind(GL_ARRAY_BUFFER);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
					sizeof(GPUOmniLight), (void*)offsetof(GPUOmniLight, pos));
				glEnableVertexAttribArray(0);
				glVertexAttribDivisor(0, 1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE,
					sizeof(GPUOmniLight), (void*)offsetof(GPUOmniLight, color));
				glEnableVertexAttribArray(1);
				glVertexAttribDivisor(1, 1);
			}

			material_template.clear();
			material_template.attachShaders("light_spot");
			material_template.options_size = 0;
			Material spotlight_material = material_template.compile();

			gl::VertexArrayObject spotlight_vao;
			gl::BufferObject spotlight_vbo;

			// Setup Spotlight vertex attribs
			{
				using scene::GPUSpotLight;
				spotlight_vao.bind();

				spotlight_vbo.bind(GL_ARRAY_BUFFER);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
					sizeof(GPUSpotLight), (void*)offsetof(GPUSpotLight, pos));
				glEnableVertexAttribArray(0);
				glVertexAttribDivisor(0, 1);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE,
					sizeof(GPUSpotLight), (void*)offsetof(GPUSpotLight, dir_exp));
				glEnableVertexAttribArray(1);
				glVertexAttribDivisor(1, 1);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
					sizeof(GPUSpotLight), (void*)offsetof(GPUSpotLight, color));
				glEnableVertexAttribArray(2);
				glVertexAttribDivisor(2, 1);
			}

			material_template.clear();
			material_template.attachShaders("fullscreen_triangle.vert", "tonemap.frag");
			material_template.options_size = 0;
			Material tonemap_material = material_template.compile();

			gl::VertexArrayObject null_vao;

			bool running = true;

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

					math::Quaternion& rot_amount = scene.transforms[wall_t_h]->rot;

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
					//x_mdelta = y_mdelta = 0.0f;
					rot_amount = math::Quaternion(math::up, -x_mdelta * MOUSE_ROT_SPEED) * rot_amount;
					rot_amount = math::Quaternion(math::right, -y_mdelta * MOUSE_ROT_SPEED) * rot_amount;

					last_mouse_pos[0] = cur_mouse_pos[0];
					last_mouse_pos[1] = cur_mouse_pos[1];

					elapsed_game_time += 1./60.;
				}

				std::vector<math::mat4> model2world_mats(scene.transforms.pool.size());
				std::vector<math::mat4> model2world_inv_mats(scene.transforms.pool.size());
				scene::calculateModel2WorldMatrices(scene.transforms,
					model2world_mats.data(), model2world_inv_mats.data());

				scene::SystemUniformBlock sys_uniforms;
				sys_uniforms.projection_mat = math::mat_transform::perspective_proj(camera.fov, render_context.aspect_ratio, camera.clip_near, camera.clip_far);
				math::mat4 world2view_mat = model2world_inv_mats[scene.transforms.getPoolIndex(camera.transform)];

				scene::renderGeometry(scene, world2view_mat, model2world_mats.data(), def_buffers, render_context, sys_uniforms);

				shading_buffers.fbo.bind(GL_DRAW_FRAMEBUFFER);
				bindGBufferTextures(def_buffers);
				math::vec4 clear_color = {0, 0, 0, 0};
				glClearBufferfv(GL_COLOR, 0, clear_color.data);
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);

				{
					std::vector<scene::GPUDirectionalLight> gpu_dirlights;
					scene::transformDirectionalLights(
						scene.lights_dir.pool, gpu_dirlights,
						world2view_mat, scene.transforms, model2world_mats.data());
					dirlight_vao.bind();
					dirlight_vbo.bind(GL_ARRAY_BUFFER);
					scene::shadeDirectionalLights(gpu_dirlights, dirlight_material, render_context, sys_uniforms);
				}

				{
					std::vector<scene::GPUOmniLight> gpu_omnilights;
					scene::transformOmniLights(
						scene.lights_omni.pool, gpu_omnilights,
						world2view_mat, scene.transforms, model2world_mats.data());
					omnilight_vao.bind();
					omnilight_vbo.bind(GL_ARRAY_BUFFER);
					scene::shadeOmniLights(gpu_omnilights, omnilight_material, render_context, sys_uniforms);
				}

				{
					std::vector<scene::GPUSpotLight> gpu_spotlights;
					scene::transformSpotLights(
						scene.lights_spot.pool, gpu_spotlights,
						world2view_mat, scene.transforms, model2world_mats.data());
					spotlight_vao.bind();
					spotlight_vbo.bind(GL_ARRAY_BUFFER);
					scene::shadeSpotLights(gpu_spotlights, spotlight_material, render_context, sys_uniforms);
				}

				for (int t = 0; t < 3; ++t) {
					glActiveTexture(GL_TEXTURE0 + t);
					glBindTexture(GL_TEXTURE_2D, 0);
				}

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				null_vao.bind();
				scene::tonemap(shading_buffers, tonemap_material, render_context);
				glBindVertexArray(0);

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
