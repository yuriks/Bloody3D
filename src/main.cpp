#include "Heatwave.hpp"
#include "gl/VertexArrayObject.hpp"
#include "gl/BufferObject.hpp"
#include "gl/Shader.hpp"
#include "gl/ShaderProgram.hpp"
#include "math/Matrix.hpp"
#include "math/MatrixTransform.hpp"

#include <iostream>
#include <fstream>

#include "gl3w.hpp"

//#define GLFW_GL3_H
#include <GL/glfw.h>

struct UniformBlock {
	math::mat4 projection_mat;
	math::mat3x4 view_model_mat;
};

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	if ((type != GL_DEBUG_TYPE_PERFORMANCE_ARB && type != GL_DEBUG_TYPE_OTHER_ARB) || severity != GL_DEBUG_SEVERITY_LOW_ARB)
		std::cerr << message << std::endl;
	if ((type != GL_DEBUG_TYPE_PERFORMANCE_ARB && type != GL_DEBUG_TYPE_OTHER_ARB) || severity == GL_DEBUG_SEVERITY_HIGH_ARB)
		__asm int 3; // Breakpoint

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
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	if (glfwOpenWindow(600, 600, 8, 8, 8, 8, 24, 0, GLFW_WINDOW) != GL_TRUE)
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

	return true;
}

HW_ALIGN_VAR_SSE const float triangle_vertex_data[3*4] = {
	-0.14f,  0.5f,  0.f, 0.f,
	 0.56f, -0.3f,  0.f, 0.f,
	-0.6f,  -0.54f, 0.f, 0.f
};

int main(int argc, char *argv[])
{
	if (!init_window())
		return 1;

	struct MeshGLState
	{
		gl::VertexArrayObject vao;
		gl::BufferObject vbo;
		gl::BufferObject ibo;
	};

	{
		MeshGLState mesh_state;

		MeshGLState& s = mesh_state;

		s.vao.bind();

		s.vbo.bind(GL_ARRAY_BUFFER);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4, triangle_vertex_data, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*4, (char*)0);
		glEnableVertexAttribArray(0);
		//glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (char*)0 + offsetof(Vertex, texcoord));
		//glEnableVertexAttribArray(1);

		//s.ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), static_cast<const void*>(indices.data()), GL_STATIC_DRAW);

		{
			gl::ShaderProgram shader_prog;

			{
				gl::Shader vert_shader(GL_VERTEX_SHADER);
				//gl::Shader geom_shader(GL_GEOMETRY_SHADER);
				gl::Shader frag_shader(GL_FRAGMENT_SHADER);

				{
					std::ifstream f("data/terrain.vert");
					vert_shader.setSource(f);
				}
				{
					//std::ifstream f("data/terrain.geom");
					//geom_shader.setSource(f);
				}
				{
					std::ifstream f("data/terrain.frag");
					frag_shader.setSource(f);
				}

				vert_shader.compile();
				//geom_shader.compile();
				frag_shader.compile();

				vert_shader.printInfoLog(std::cout);
				//geom_shader.printInfoLog(std::cout);
				frag_shader.printInfoLog(std::cout);

				shader_prog.attachShader(vert_shader);
				//shader_prog.attachShader(geom_shader);
				shader_prog.attachShader(frag_shader);

				shader_prog.bindAttribute(0, "in_Position");
				//shader_prog.bindAttribute(1, "in_TexCoord");
				glBindFragDataLocation(shader_prog, 0, "out_Color");

				shader_prog.link();
				shader_prog.printInfoLog(std::cout);
			}

			shader_prog.use();


			bool running = true;

			glClearColor(0.2f, 0.2f, 0.2f, 1.f);
			//glEnable(GL_DEPTH_TEST);
			//glEnable(GL_CULL_FACE);
			//glEnable(GL_DEPTH_CLAMP);
			glEnable(GL_MULTISAMPLE);
			glFrontFace(GL_CW);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			UniformBlock uniforms;
			uniforms.projection_mat = math::mat_transform::perspective_proj(35.f, 800.f/600.f, 0.1f, 500.f);

			gl::BufferObject ubo;
			ubo.bind(GL_UNIFORM_BUFFER);

			GLuint uniform_block_index = glGetUniformBlockIndex(shader_prog, "UniformBlock");

			glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBlock), &uniforms, GL_STREAM_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
			glUniformBlockBinding(shader_prog, uniform_block_index, 0);

			/*
			vec3 cam_rot(0.f, 0.f, 0.f);
			vec3 cam_pos(0.f, 0.f, 0.f);

			vec3 *pos = &cam_pos;

			std::cout <<
				"Teclas:\n"
				"  WASD - Movimentacao\n"
				"  R/F - Subir/descer\n"
				"  OKL; - Rotaciona camera\n"
				"\n"
				"  1/2 - Mudar nivel de detalhe fixo.\n" << std::endl;
			*/

			double elapsed_game_time = 0.;
			double elapsed_real_time = 0.;
			double last_frame_time;

			//bool prev_keys[2] = {false};

			//int lod_level = 0;

			math::mat3x4 view;
			float rot_amount = 0.f;
			float move_amount = 0.f;

			while (running)
			{
				double frame_start = glfwGetTime();

				int i;
				for (i = 0; elapsed_game_time < elapsed_real_time && i < 5; ++i)
				{
					/*
					if (glfwGetKey('O')) cam_rot -= vec3(0.02f, 0.f, 0.f); // Y - O
					if (glfwGetKey('L')) cam_rot += vec3(0.02f, 0.f, 0.f); // I - L
					if (glfwGetKey('K')) cam_rot -= vec3(0.f, 0.02f, 0.f); // E - K
					if (glfwGetKey(';')) cam_rot += vec3(0.f, 0.02f, 0.f); // O - ;

					// Inverse of view_rot below
					const mat3x4 view_rot_move = concatTransform(mat_transform::rotate(vec3(0.f, 1.f, 0.f), -cam_rot.getY()), mat_transform::rotate(vec3(1.f, 0.f, 0.f), -cam_rot.getX()));

					vec3 x_axis = transform(view_rot_move, vec3(1.f, 0.f, 0.f));
					vec3 y_axis = transform(view_rot_move, vec3(0.f, 1.f, 0.f));
					vec3 z_axis = transform(view_rot_move, vec3(0.f, 0.f, 1.f));

					if (glfwGetKey('A')) (*pos) -= x_axis * 0.25f; // A - A
					if (glfwGetKey('D')) (*pos) += x_axis * 0.25f; // S - D
					if (glfwGetKey('R')) (*pos) += y_axis * 0.25f; // P - R
					if (glfwGetKey('F')) (*pos) -= y_axis * 0.25f; // T - F
					if (glfwGetKey('W')) (*pos) += z_axis * 0.25f; // W - W
					if (glfwGetKey('S')) (*pos) -= z_axis * 0.25f; // R - S

					bool key = glfwGetKey('1') != 0;
					if (key && !prev_keys[0])
					{
						if (lod_level > 0)
							glUniform1i(u_LodLevel, --lod_level);
					}
					prev_keys[0] = key;

					key = glfwGetKey('2') != 0;
					if (key && !prev_keys[1])
					{
						//if (lod_level > 0)
						glUniform1i(u_LodLevel, ++lod_level);
					}
					prev_keys[1] = key;
					*/

					rot_amount += 0.0105f;
					move_amount += 0.005;
					view = math::concatTransform(math::mat_transform::translate3x4(math::vec3(0.f, 0.f, move_amount)), math::mat_transform::rotate(math::vec3(0.f, 1.f, 0.f), rot_amount));

					elapsed_game_time += 1./60.;
				}

				//const mat3x4 view_rot = concatTransform(mat_transform::rotate(vec3(1.f, 0.f, 0.f), cam_rot.getX()), mat_transform::rotate(vec3(0.f, 1.f, 0.f),  cam_rot.getY()));
				//view = concatTransform(view_rot, mat_transform::translate3x4(cam_pos * -1.f));
				//view = mat_transform::look_at(make_vec(0.f, 1.f, 0.f), cam_pos, obj_pos);

				/*
				Light light_transformed;
				{
					light_transformed.color = lights.color;
					light_transformed.direction = vec3(transform(view, vec4(lights.direction)));
				}
				*/

				uniforms.view_model_mat = view;

				ubo.bind(GL_UNIFORM_BUFFER);
				glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBlock), &uniforms, GL_STREAM_DRAW);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				mesh_state.vao.bind();
				glDrawArrays(GL_TRIANGLES, 0, 3);
				//glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (char*)0);

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
