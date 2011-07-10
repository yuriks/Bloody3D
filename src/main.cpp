#include "Matrix.hpp"
#include "Vector.hpp"
#include "MatrixTransform.hpp"
#include "gl/VertexArrayObject.hpp"
#include "gl/BufferObject.hpp"
#include "gl/Shader.hpp"
#include "gl/ShaderProgram.hpp"
#include "gl/Texture.hpp"
#include "image/ImageLoader.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/ObjLoader.hpp"

#include <iostream>
#include <fstream>

#include "gl3w.hpp"

//#define GLFW_GL3_H
#include <GL/glfw.h>

using namespace math;

struct Light
{
	union {
		vec3 position;
		float pad1[4]; // Enforce alignment
	};
	union {
		vec3 color;
		float pad2[4];
	};
};
static_assert(sizeof(Light) == (4+4)*4, "OOPS!");
static_assert(offsetof(Light, position) == 0, "OOPS!");
static_assert(offsetof(Light, color) == 4*4, "OOPS!");

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	std::cout << message << std::endl;
}


// Eh isso o que acontece quando se procrastina demais
int main(int argc, char *argv[])
{
	if (glfwInit() != GL_TRUE)
	{
		char tmp;
		std::cerr << "Failed to initialize GLFW." << std::endl;
		std::cin >> tmp;
		return 1;
	}

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (glfwOpenWindow(800, 600, 8, 8, 8, 8, 24, 8, GLFW_WINDOW) != GL_TRUE)
	{
		char tmp;
		std::cerr << "Failed to open window." << std::endl;
		std::cin >> tmp;
		return 1;
	}

	glfwSwapInterval(1);

	if (gl3wInit() != 0) {
		char tmp;
		std::cerr << "Failed to initialize gl3w." << std::endl;
		std::cin >> tmp;
		return 1;
	} else if (!gl3wIsSupported(3, 3)) {
		char tmp;
		std::cerr << "OpenGL 3.3 not supported." << std::endl;
		std::cin >> tmp;
		return 1;
	}

	if (glDebugMessageCallbackARB) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
		glDebugMessageCallbackARB(debug_callback, 0);
	}

	// Gambiarras sem tempo :P
	static const int NUM_MESHES = 3;

	std::ifstream f("data/panel-beams.obj");
	Mesh panel_beams = load_obj(f);
	f.close();

	{
		gl::VertexArrayObject vao[NUM_MESHES];
		gl::BufferObject vbo[NUM_MESHES];
		gl::BufferObject ibo[NUM_MESHES];

		for (unsigned int i = 0; i < NUM_MESHES; ++i)
		{
			vao[i].bind();

			vbo[i].bind(GL_ARRAY_BUFFER);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * panel_beams.sub_meshes[i].vertices.size(),
				static_cast<const void*>(&panel_beams.sub_meshes[i].vertices[0]), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + offsetof(Vertex, position));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE,  sizeof(Vertex), (char*)0 + offsetof(Vertex, normal));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE,  sizeof(Vertex), (char*)0 + offsetof(Vertex, tex_coord));
			glEnableVertexAttribArray(2);

			ibo[i].bind(GL_ELEMENT_ARRAY_BUFFER);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short) * panel_beams.sub_meshes[i].indices.size(),
				static_cast<const void*>(&panel_beams.sub_meshes[i].indices[0]), GL_STATIC_DRAW);
		}

		gl::ShaderProgram shader_prog;

		{
			gl::Shader vert_shader(GL_VERTEX_SHADER);
			gl::Shader frag_shader(GL_FRAGMENT_SHADER);

			{
				std::ifstream f("data/test.vert");
				vert_shader.setSource(f);
			}
			{
				std::ifstream f("data/test.frag");
				frag_shader.setSource(f);
			}

			vert_shader.compile();
			frag_shader.compile();

			vert_shader.printInfoLog(std::cout);
			frag_shader.printInfoLog(std::cout);

			shader_prog.attachShader(vert_shader);
			shader_prog.attachShader(frag_shader);

			shader_prog.bindAttribute(0, "in_Position");
			shader_prog.bindAttribute(1, "in_Normal");
			shader_prog.bindAttribute(2, "in_TexCoord");
			glBindFragDataLocation(shader_prog, 0, "out_Color");

			shader_prog.link();
			shader_prog.printInfoLog(std::cout);
		}

		shader_prog.use();

		gl::Texture diffuse_map;
		gl::Texture normal_map;

		{
			image::Image img;
			std::ifstream f("assets/diffuse_map.png", std::ios::in | std::ios::binary);
			image::Image::loadPNGFileRGBA8(img, f);

			diffuse_map.bind(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			diffuse_map.width = img.getWidth();
			diffuse_map.height = img.getHeight();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, diffuse_map.width, diffuse_map.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getData());
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		{
			image::Image img;
			std::ifstream f("assets/normal_map.png", std::ios::in | std::ios::binary);
			image::Image::loadPNGFileRGBA8(img, f);

			normal_map.bind(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			normal_map.width = img.getWidth();
			normal_map.height = img.getHeight();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, normal_map.width, normal_map.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getData());
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		bool running = true;

		using mat_transform::scale;
		using mat_transform::rotate;
		using mat_transform::translate;

		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_CLAMP);

		glActiveTexture(GL_TEXTURE0);
		diffuse_map.bind(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		normal_map.bind(GL_TEXTURE_2D);

		glUniform1i(shader_prog.getUniformLocation("u_DiffuseMap"), 0);
		glUniform1i(shader_prog.getUniformLocation("u_NormalMap"), 1);

		Light lights_base[4];
		lights_base[0].position = make_vec(1.f, 1.f, -1.f);
		lights_base[0].color = make_vec(1.f, 1.f, 1.f);
		lights_base[1].color = make_vec(0.f, 0.f, 0.f);
		lights_base[2].color = make_vec(0.f, 0.f, 0.f);
		lights_base[3].color = make_vec(0.f, 0.f, 0.f);

		gl::BufferObject ubo_lights;
		ubo_lights.bind(GL_UNIFORM_BUFFER);

		GLuint uniform_block_index = glGetUniformBlockIndex(shader_prog, "LightBlock");

		glBufferData(GL_UNIFORM_BUFFER, sizeof(lights_base), lights_base, GL_STREAM_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_lights);
		glUniformBlockBinding(shader_prog, uniform_block_index, 0);

		mat4 proj = mat_transform::perspective_proj(35.f, 800.f/600.f, 0.1f, 100.f);
		//mat4 proj = mat_transform::orthographic_proj(-2.f * (800.f/600.f), 2.f * (800.f/600.f), -2.f, 2.f, 1.f, 4.f);

		GLuint in_ViewModelMat = shader_prog.getUniformLocation("in_ViewModelMat");
		GLuint in_ProjMat = shader_prog.getUniformLocation("in_ProjMat");

		glUniformMatrix4fv(in_ProjMat, 1, false, &proj.data[0]);

		vec3 obj_pos = {{ 0.f, 0.f, 0.f }};
		vec3 obj_rot = {{ 0.f, 0.f, 0.f }};
		vec3 cam_pos = {{ 0.f, 0.f, -1.f }};

		vec3 *pos = &cam_pos;

		std::cout <<
			"Teclas:\n"
			"  A/D - Movimentacao eixo -X/+X\n"
			"  F/R - Movimentacao eixo -Y/+Y\n"
			"  S/W - Movimentacao eixo -Z/+Z\n"
			"  O/L - K/; - I/P - Rotaciona objeto nos eixos X/Y/Z\n"
			"\n"
			"  1/2 - Projecao ortografica/perspectiva.\n"
			"  3/4/5 - Move Camera/Objeto/Luz.\n"
			"  6 - Muda a cor da luz. (Confuso.)\n"
			"  7/8 - Desliga/Liga wireframe.\n" << std::endl;

		double elapsed_game_time = 0.;
		double elapsed_real_time = 0.;
		double last_frame_time;

		mat4 view;
		Light lights[4];
		bool wire;

		while (running)
		{
			double frame_start = glfwGetTime();

			int i;
			for (i = 0; elapsed_game_time < elapsed_real_time && i < 5; ++i)
			{
				//game_manager.update();

				if (glfwGetKey('1')) proj = mat_transform::perspective_proj(35.f, 800.f/600.f, 0.1f, 100.f);
				if (glfwGetKey('2')) proj = mat_transform::orthographic_proj(-2.f * (800.f/600.f), 2.f * (800.f/600.f), -2.f, 2.f, 0.1f, 100.f);
				glUniformMatrix4fv(in_ProjMat, 1, false, &proj.data[0]);

				if (glfwGetKey('3')) pos = &cam_pos;
				if (glfwGetKey('4')) pos = &obj_pos;
				if (glfwGetKey('5')) pos = &lights_base[0].position;
				if (glfwGetKey('6')) pos = &lights_base[0].color;
				if (glfwGetKey('7')) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); wire = false; }
				if (glfwGetKey('8')) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); wire = true; }

				if (glfwGetKey('A')) (*pos)[0] -= 0.05f; // A - A
				if (glfwGetKey('D')) (*pos)[0] += 0.05f; // S - D
				if (glfwGetKey('F')) (*pos)[1] -= 0.05f; // T - F
				if (glfwGetKey('R')) (*pos)[1] += 0.05f; // P - R
				if (glfwGetKey('W')) (*pos)[2] += 0.05f; // W - W
				if (glfwGetKey('S')) (*pos)[2] -= 0.05f; // R - S

				if (glfwGetKey('O')) obj_rot[0] -= 0.05f; // Y - O
				if (glfwGetKey('L')) obj_rot[0] += 0.05f; // I - L
				if (glfwGetKey('K')) obj_rot[1] -= 0.05f; // E - K
				if (glfwGetKey(';')) obj_rot[1] += 0.05f; // O - ;
				if (glfwGetKey('I')) obj_rot[2] -= 0.05f; // U - I
				if (glfwGetKey('P')) obj_rot[2] += 0.05f; // ; - P

				view = mat_transform::look_at(make_vec(0.f, 1.f, 0.f), cam_pos, obj_pos);

				for	(int i = 0; i < 4; ++i)
				{
					lights[i].position = vec::euclidean(view * vec::homogeneous(lights_base[i].position));
					if (wire)
						lights[i].color = make_vec(9999.f, 9999.f, 9999.f);
					else
						lights[i].color = lights_base[i].color;
				}

				ubo_lights.bind(GL_UNIFORM_BUFFER);
				glBufferData(GL_UNIFORM_BUFFER, sizeof(lights), lights, GL_STREAM_DRAW);

				elapsed_game_time += 1./60.;
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			static const vec3 positions[12] = {
				{{ 2.f, 0.f, 0.f }}, {{ 0.f, 1.f, 0.f }},
				{{ 0.f, 0.f, 2.f }}, {{ 0.f, 1.f, 0.f }},
				{{-2.f, 0.f, 0.f }}, {{ 0.f, 1.f, 0.f }},
				{{ 0.f, 0.f,-2.f }}, {{ 0.f, 1.f, 0.f }},
				{{ 0.f, 2.f, 0.f }}, {{ 1.f, 0.f, 0.f }},
				{{ 0.f,-2.f, 0.f }}, {{ 1.f, 0.f, 0.f }}
			};

			static const float angles[6] = {
				1.f, 2.f, 3.f, 0.f, -1.f, 1.f
			};

			for (int j = 0; j < 6; ++j)
			{
				for (int i = 0; i < NUM_MESHES; ++i)
				{

					mat4 model = translate(obj_pos) *
						rotate(vec::unit(make_vec(0.f, 1.f, 0.f)), obj_rot[1]) *
						rotate(vec::unit(make_vec(1.f, 0.f, 0.f)), obj_rot[0]) *
						rotate(vec::unit(make_vec(0.f, 0.f, 1.f)), obj_rot[2]) *
						scale(make_vec(1.f/3.f, 1.f/3.f, 1.f/3.f)) *
						translate(positions[j*2]) * rotate(positions[j*2+1], angles[j] * 1.5707963267948966192313216916397514420f);

					mat4 viewmodel = view * model;

					glUniformMatrix4fv(in_ViewModelMat, 1, false, &viewmodel.data[0]);

					vao[i].bind();
					glDrawElements(GL_TRIANGLES, panel_beams.sub_meshes[i].indices.size(), GL_UNSIGNED_SHORT, (char*)0);
				}
			}

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
