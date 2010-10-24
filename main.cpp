#include "Matrix.h"
#include "Vector.h"
#include "MatrixTransform.h"
#include <iostream>
#include <vector>
#include <cmath>

#include "gl3w.h"

//#define GLFW_GL3_H
#include <GL/glfw.h>

static const char* vert_shader_src =
"#version 150\n"
"// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1\n"
"in  vec4 in_Position;\n"
"in  vec4 in_Color;\n"
"uniform mat4 in_Proj;\n"
"\n"
"// We output the ex_Color variable to the next shader in the chain\n"
"out vec4 ex_Color;\n"
"\n"
"void main(void) {\n"
"    gl_Position = in_Proj * in_Position;\n"
"    ex_Color = in_Color;\n"
"}\n";

static const char* frag_shader_src =
"#version 150\n"
"\n"
"in  vec4 ex_Color;\n"
"out vec4 gl_FragColor;\n"
"\n"
"void main(void) {\n"
"    // Pass through our original color with full opacity.\n"
"    gl_FragColor = ex_Color;\n"
"}\n";

struct vertex_data {
	float x, y, z, w;
	float r, g, b, a;
};
static_assert (sizeof(vertex_data) == sizeof(float)*8, "Oops. Padding.");

static const vertex_data data[3] = {
	{-.5f,  .5f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f},
	{ .5f, -.5f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f},
	{-.5f, -.5f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f}
};
static_assert (sizeof(data) == sizeof(vertex_data)*3, "Oops. Padding.");

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

	std::cout << gl3wInit() << std::endl;
	std::cout << gl3wIsSupported(3, 3) << std::endl;

	if (glDebugMessageCallbackARB) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
		glDebugMessageCallbackARB(debug_callback, 0);
	}

	GLuint vao_id;
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	GLuint vbo_id;
	glGenBuffers(1, &vbo_id);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data)*3, static_cast<const void*>(&data), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(float), (float*)0 + 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE,  8*sizeof(float), (float*)0 + 4);
	glEnableVertexAttribArray(1);

	GLuint vert_shader_id, frag_shader_id;
	vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
	frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vert_shader_id, 1, &vert_shader_src, 0);
	glShaderSource(frag_shader_id, 1, &frag_shader_src, 0);

	glCompileShader(vert_shader_id);
	glCompileShader(frag_shader_id);

	GLint vert_log_size, frag_log_size;
	glGetShaderiv(vert_shader_id, GL_INFO_LOG_LENGTH, &vert_log_size);
	glGetShaderiv(frag_shader_id, GL_INFO_LOG_LENGTH, &frag_log_size);
	std::vector<char> log_buf(std::max(vert_log_size, frag_log_size));

	glGetShaderInfoLog(vert_shader_id, log_buf.size(), 0, &log_buf[0]);
	std::cout << &log_buf[0];
	glGetShaderInfoLog(frag_shader_id, log_buf.size(), 0, &log_buf[0]);
	std::cout << &log_buf[0];


	GLuint shader_prog_id;
	shader_prog_id = glCreateProgram();

	glAttachShader(shader_prog_id, vert_shader_id);
	glAttachShader(shader_prog_id, frag_shader_id);

	glBindAttribLocation(shader_prog_id, 0, "in_Position");
	glBindAttribLocation(shader_prog_id, 1, "in_Color");

	glLinkProgram(shader_prog_id);

	glDeleteShader(vert_shader_id);
	glDeleteShader(frag_shader_id);

	glUseProgram(shader_prog_id);

	bool running = true;

	vec3 s = {600./800., 1.f, 1.f};
	mat4 m = mat_transform::scale(s);

	while (running) {
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUniformMatrix4fv(glGetUniformLocation(shader_prog_id, "in_Proj"), 1, false, &m.data[0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers();

		running = glfwGetWindowParam(GLFW_OPENED) != 0;
	}

	glDeleteProgram(shader_prog_id);

	glDeleteBuffers(1, &vbo_id);
	glDeleteVertexArrays(1, &vao_id);

	glfwTerminate();

	return 0;
}
