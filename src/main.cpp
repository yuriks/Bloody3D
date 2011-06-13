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

int main(int argc, char *argv[])
{
	glfwInit();

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwOpenWindow(800, 600, 8, 8, 8, 8, 24, 8, GLFW_WINDOW);

	if (gl3wInit() != 0) {
		std::cerr << "Failed to initialize gl3w." << std::endl;
	} else if (!gl3wIsSupported(3, 3)) {
		std::cerr << "OpenGL 3.3 not supported." << std::endl;
	}

	if (glDebugMessageCallbackARB) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
		glDebugMessageCallbackARB(debug_callback, 0);
	}

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
		float ang = 0.f;

		using mat_transform::scale;
		using mat_transform::rotate;
		using mat_transform::translate;

		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

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

		//mat4 projection = mat_transform::perspective(45.f, 600.f/800.f, 1.f, 10000.f);
		mat4 projview = scale(make_vec(600./800., 1.f, 1.f));

		GLuint in_ModelMat = shader_prog.getUniformLocation("in_ModelMat");
		GLuint in_ProjViewMat = shader_prog.getUniformLocation("in_ProjViewMat");

		while (running) {
			mat4 model = rotate(vec::unit(make_vec(0.f, 1.f, -0.2f)), ang) * scale(make_vec(1.f/3.f, 1.f/3.f, 1.f/3.f));

			Light lights[4];

			for	(int i = 0; i < 4; ++i)
			{
				lights[i].position = vec::euclidean(projview * vec::homogeneous(lights_base[i].position));
				lights[i].color = lights_base[i].color;
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUniformMatrix4fv(in_ModelMat, 1, false, &model.data[0]);
			glUniformMatrix4fv(in_ProjViewMat, 1, false, &projview.data[0]);
			ubo_lights.bind(GL_UNIFORM_BUFFER);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(lights), lights, GL_STREAM_DRAW);

			for (int i = 0; i < NUM_MESHES; ++i)
			{
				vao[i].bind();
				glDrawElements(GL_TRIANGLES, panel_beams.sub_meshes[i].indices.size(), GL_UNSIGNED_SHORT, (char*)0);
			}

			glfwSwapBuffers();

			ang += 0.01f;

			running = glfwGetWindowParam(GLFW_OPENED) != 0;
		}
	}

	glfwTerminate();

	return 0;
}
