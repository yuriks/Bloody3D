#include "Matrix.hpp"
#include "Vector.hpp"
#include "MatrixTransform.hpp"
#include "gl/VertexArrayObject.hpp"
#include "gl/BufferObject.hpp"
#include "gl/Shader.hpp"
#include "gl/ShaderProgram.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/ObjLoader.hpp"

#include <iostream>
#include <fstream>

#include "gl3w.hpp"

//#define GLFW_GL3_H
#include <GL/glfw.h>

using namespace math;

static const char* vert_shader_src =
"#version 330\n"
"// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1\n"
"in  vec3 in_Position;\n"
"in  vec3 in_Normal;\n"
"uniform mat4 in_Proj;\n"
"\n"
"// We output the ex_Color variable to the next shader in the chain\n"
"out vec4 ex_Color;\n"
"\n"
"void main(void) {\n"
"    gl_Position = in_Proj * vec4(in_Position.xyz, 1.0);\n"
"    ex_Color = vec4(0.5 + in_Normal / 2, 1.0);\n"
"}\n";

static const char* frag_shader_src =
"#version 330\n"
"\n"
"in  vec4 ex_Color;\n"
"out vec4 out_Color;\n"
"\n"
"void main(void) {\n"
"    // Pass through our original color with full opacity.\n"
"    out_Color = ex_Color;\n"
"}\n";

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

	std::ifstream f("assets/panel-beams.obj");
	Mesh panel_beams = load_obj(f);
	f.close();

	{
		gl::VertexArrayObject vao[3];
		gl::BufferObject vbo[3];
		gl::BufferObject ibo[3];

		for (unsigned int i = 0; i < 3; ++i)
		{
			vao[i].bind();

			vbo[i].bind(GL_ARRAY_BUFFER);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * panel_beams.sub_meshes[i].vertices.size(),
				static_cast<const void*>(&panel_beams.sub_meshes[i].vertices[0]), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + offsetof(Vertex, position));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE,  sizeof(Vertex), (char*)0 + offsetof(Vertex, normal));
			glEnableVertexAttribArray(1);

			ibo[i].bind(GL_ELEMENT_ARRAY_BUFFER);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short) * panel_beams.sub_meshes[i].indices.size(),
				static_cast<const void*>(&panel_beams.sub_meshes[i].indices[0]), GL_STATIC_DRAW);
		}

		gl::ShaderProgram shader_prog;

		{
			gl::Shader vert_shader(GL_VERTEX_SHADER);
			gl::Shader frag_shader(GL_FRAGMENT_SHADER);

			vert_shader.setSource(vert_shader_src);
			frag_shader.setSource(frag_shader_src);

			vert_shader.compile();
			frag_shader.compile();

			vert_shader.printInfoLog(std::cout);
			frag_shader.printInfoLog(std::cout);

			shader_prog.attachShader(vert_shader);
			shader_prog.attachShader(frag_shader);

			shader_prog.bindAttribute(0, "in_Position");
			shader_prog.bindAttribute(1, "in_Normal");
			glBindFragDataLocation(shader_prog, 0, "out_Color");

			shader_prog.link();
			shader_prog.printInfoLog(std::cout);
		}

		shader_prog.use();

		bool running = true;
		float ang = 0.f;

		using mat_transform::scale;
		using mat_transform::rotate;
		using mat_transform::translate;

		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		while (running) {
			mat4 m = scale(make_vec(600./800. /4.f, 1.f/4.f, 1.f/4.f)) * translate(make_vec(-.5f, 0.f, 0.f)) * rotate(vec::unit(make_vec(0.f, 1.f, 0.2f)), ang);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUniformMatrix4fv(shader_prog.getUniformLocation("in_Proj"), 1, false, &m.data[0]);

			for (int i = 0; i < 3; ++i)
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
