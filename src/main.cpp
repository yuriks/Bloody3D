// Work-around stupid windows.h defines
#define min min
#define max max

#include "Matrix.hpp"
#include "Vector.hpp"
#include "MatrixTransform.hpp"
#include "gl/VertexArrayObject.hpp"
#include "gl/BufferObject.hpp"
#include "gl/Shader.hpp"
#include "gl/ShaderProgram.hpp"
#include "gl/Texture.hpp"
#include "image/ImageLoader.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <cassert>

#include "gl3w.hpp"

//#define GLFW_GL3_H
#include <GL/glfw.h>

#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14159265358979323846264338327950288419716939937510f

using namespace math;

struct Light
{
	union {
		vec3 direction;
		float pad1[4]; // Enforce alignment
	};
	union {
		vec3 color;
		float pad2[4];
	};
};
static_assert(sizeof(Light) == (4+4)*4, "OOPS!");
static_assert(offsetof(Light, direction) == 0, "OOPS!");
static_assert(offsetof(Light, color) == 4*4, "OOPS!");

struct Vertex
{
	vec2 pos;
	vec2 texcoord;
};

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	if (severity != GL_DEBUG_SEVERITY_LOW_ARB)
		std::cerr << message << std::endl;
	if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
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
	if (glfwOpenWindow(800, 600, 8, 8, 8, 8, 24, 0, GLFW_WINDOW) != GL_TRUE)
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

float lerp(float a, float b, float x)
{
	return a * (1.f - x) + b * x;
}

float clamp(float min, float x, float max)
{
	return std::min(std::max(min, x), max);
}

vec3 lookup_value(const std::vector<unsigned char>& lut, float pos)
{
	int lut_entries = lut.size() / 4;
	float table_pos = clamp(0.f, pos, 1.f) * float(lut_entries - 1);
	int index_a = (int)std::floor(table_pos);
	int index_b = (int)std::ceil(table_pos);
	float mix = table_pos - std::floor(table_pos);

	if (index_a < 0)
		index_a = 0;
	if (index_a >= lut_entries)
		index_a = lut_entries - 1;

	if (index_b < 0)
		index_b = 0;
	if (index_b >= lut_entries)
		index_b = lut_entries - 1;

	vec3 col;
	for (int i = 0; i < 3; ++i)
	{
		col[i] = lerp(lut[index_a*4 + i] / 255.f, lut[index_b*4 + i] / 255.f, mix);
	}

	return col;
}

int main(int argc, char *argv[])
{
	if (!init_window())
		return 1;

	gl::Texture color_lookup_tex;
	{
		image::Image image;
		{
			std::ifstream image_file("data/color-lookup.png", std::ios::in | std::ios::binary);
			image::GrayscaleImage::loadPNGFileRGBA8(image, image_file);
		}

		unsigned int width = image.getWidth();

		glActiveTexture(GL_TEXTURE0);
		color_lookup_tex.bind(GL_TEXTURE_1D);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getData());
	}

	std::vector<unsigned char> light_lookup;
	{
		image::Image image;
		{
			std::ifstream image_file("data/light-lookup.png", std::ios::in | std::ios::binary);
			image::GrayscaleImage::loadPNGFileRGBA8(image, image_file);
		}

		unsigned int width = image.getWidth();
		light_lookup.resize(width * 4);
		std::copy(image.getData(), image.getData() + width*4, light_lookup.begin());
	}

	unsigned int width, height;
	gl::Texture terrain_tex;
	{
		image::GrayscaleImage image;
		{
			std::ifstream image_file("data/terrain16.png", std::ios::in | std::ios::binary);
			image::GrayscaleImage::loadPNGFileGray16(image, image_file);
		}

		width = image.getWidth();
		height = image.getHeight();
		unsigned short* data = reinterpret_cast<unsigned short*>(image.getData());

		for (unsigned int y = 3; y < height-3; ++y) {
			for (unsigned int x = 3; x < width-3; ++x) {
				long long v = 0;
				for (int yy = -3; yy <= 3; ++yy) {
					for (int xx = -3; xx <= 3; ++xx) {
						v += int(data[(y+yy)*width + x+xx]);
					}
				}
				data[y*width + x] = (unsigned short)(v / 49);
			}
		}

		glActiveTexture(GL_TEXTURE1);
		terrain_tex.bind(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, image.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	static const float SPACING = .1f;
	std::vector<Vertex> grid;
	grid.resize(width*height);
	{
		unsigned int i = 0;
		for (unsigned int y = 0; y < height; ++y)
		{
			for (unsigned int x = 0; x < width; ++x)
			{
				Vertex v;
				v.pos = make_vec(x * SPACING, y * SPACING);
				v.texcoord = make_vec((float)x / width, (float)y / width);
				grid[i++] = v;
			}
		}
		assert(i == grid.size());
	}

	std::vector<uint32_t> indices;
	indices.resize((width-1)*(height-1)*3*2);
	{
		unsigned int i = 0;
		for (unsigned int y = 0; y < height-1; ++y)
		{
			for (unsigned int x = 0; x < width-1; ++x)
			{
				// A--B
				// | /|
				// |/ |
				// C--D
				unsigned int a = y*width + x;
				unsigned int b = a + 1;
				unsigned int c = a + width;
				unsigned int d = c + 1;

				// First tri
				indices[i++] = a;
				indices[i++] = b;
				indices[i++] = c;

				// Second tri
				indices[i++] = b;
				indices[i++] = d;
				indices[i++] = c;
			}
		}
		assert(i == indices.size());
	}

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
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * grid.size(), static_cast<const void*>(grid.data()), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + offsetof(Vertex, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (char*)0 + offsetof(Vertex, texcoord));
		glEnableVertexAttribArray(1);

		s.ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), static_cast<const void*>(indices.data()), GL_STATIC_DRAW);

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
				shader_prog.bindAttribute(1, "in_TexCoord");
				glBindFragDataLocation(shader_prog, 0, "out_Color");

				shader_prog.link();
				shader_prog.printInfoLog(std::cout);
			}

			shader_prog.use();


			bool running = true;

			using mat_transform::scale;
			using mat_transform::rotate;
			using mat_transform::translate;

			glClearColor(0.2f, 0.2f, 0.2f, 1.f);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			//glEnable(GL_DEPTH_CLAMP);
			glEnable(GL_MULTISAMPLE);
			glFrontFace(GL_CCW);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			float light_rot = -15.f;
			Light lights;
			//lights.direction = make_vec(2.12f, 2.24f, 2.52f);
			lights.direction = vec::unit(make_vec(-2.f, -3.f, -2.5f));
			//lights.color = make_vec(1.f, 1.f, 1.f);

			gl::BufferObject ubo_lights;
			ubo_lights.bind(GL_UNIFORM_BUFFER);

			GLuint uniform_block_index = glGetUniformBlockIndex(shader_prog, "LightBlock");

			glBufferData(GL_UNIFORM_BUFFER, sizeof(lights), &lights, GL_STREAM_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_lights);
			glUniformBlockBinding(shader_prog, uniform_block_index, 0);

			mat4 proj = mat_transform::perspective_proj(35.f, 800.f/600.f, 0.1f, 500.f);

			GLuint in_ViewModelMat = shader_prog.getUniformLocation("in_ViewModelMat");
			GLuint in_ProjMat = shader_prog.getUniformLocation("in_ProjMat");
			GLuint u_Heightmap = shader_prog.getUniformLocation("u_Heightmap");
			GLuint u_ColorLookup = shader_prog.getUniformLocation("u_ColorLookup");
			GLuint u_LodLevel = shader_prog.getUniformLocation("u_LodLevel");

			glUniformMatrix4fv(in_ProjMat, 1, false, &proj.data[0]);
			glUniform1i(u_ColorLookup, 0);
			glUniform1i(u_Heightmap, 1);
			glUniform1i(u_LodLevel, 0);

			vec3 cam_rot = {{ 0.f, 0.f, 0.f }};
			vec3 cam_pos = {{ 0.f, 0.f, 0.f }};

			vec3 *pos = &cam_pos;

			std::cout <<
				"Teclas:\n"
				"  WASD - Movimentacao\n"
				"  R/F - Subir/descer\n"
				"  OKL; - Rotaciona camera\n"
				"\n"
				"  1/2 - Mudar nivel de detalhe fixo.\n" << std::endl;

			double elapsed_game_time = 0.;
			double elapsed_real_time = 0.;
			double last_frame_time;

			bool prev_keys[2] = {false};

			int lod_level = 0;

			mat4 view;

			while (running)
			{
				double frame_start = glfwGetTime();

				int i;
				for (i = 0; elapsed_game_time < elapsed_real_time && i < 5; ++i)
				{
					if (glfwGetKey('O')) cam_rot[0] -= 0.02f; // Y - O
					if (glfwGetKey('L')) cam_rot[0] += 0.02f; // I - L
					if (glfwGetKey('K')) cam_rot[1] -= 0.02f; // E - K
					if (glfwGetKey(';')) cam_rot[1] += 0.02f; // O - ;

					// Inverse of view_rot below
					const mat4 view_rot_move = mat_transform::rotate(make_vec(0.f, 1.f, 0.f), -cam_rot[1]) * mat_transform::rotate(make_vec(1.f, 0.f, 0.f), -cam_rot[0]);

					vec3 x_axis = vec::euclidean(view_rot_move * make_vec(1.f, 0.f, 0.f, 1.f));
					vec3 y_axis = vec::euclidean(view_rot_move * make_vec(0.f, 1.f, 0.f, 1.f));
					vec3 z_axis = vec::euclidean(view_rot_move * make_vec(0.f, 0.f, 1.f, 1.f));

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

					light_rot += 0.05f;
					if (light_rot >= 180.f + 15.f)
						light_rot -= (180.f + 30.f);
					lights.direction = vec::euclidean(mat_transform::rotate(make_vec(0.f, 1.f, 0.f), 0.4) * (mat_transform::rotate(make_vec(1.f, 0.f, 0.f), light_rot * M_PI/180.f) * make_vec(0.f, 0.f, -1.f, 1.f)));
					lights.color = lookup_value(light_lookup, (light_rot + 15.f) / 210.f);

					elapsed_game_time += 1./60.;
				}

				const mat4 view_rot = mat_transform::rotate(make_vec(1.f, 0.f, 0.f),  cam_rot[0]) * mat_transform::rotate(make_vec(0.f, 1.f, 0.f),  cam_rot[1]);
				view = view_rot * mat_transform::translate(cam_pos * -1.f);
				//view = mat_transform::look_at(make_vec(0.f, 1.f, 0.f), cam_pos, obj_pos);

				Light light_transformed;
				{
					light_transformed.color = lights.color;
					vec4 tmp = view * make_vec(lights.direction[0], lights.direction[1], lights.direction[2], 0.f);
					light_transformed.direction = make_vec(tmp[0], tmp[1], tmp[2]);
				}

				ubo_lights.bind(GL_UNIFORM_BUFFER);
				glBufferData(GL_UNIFORM_BUFFER, sizeof(lights), &light_transformed, GL_STREAM_DRAW);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glUniformMatrix4fv(in_ViewModelMat, 1, false, &view.data[0]);
				mesh_state.vao.bind();
				glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (char*)0);

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
