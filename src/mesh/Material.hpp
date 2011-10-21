#pragma once

#include "gl/Shader.hpp"
#include "gl/ShaderProgram.hpp"
#include "Heatwave.hpp"
#include <GL3/gl3.h>
#include <memory>
#include <array>

namespace gl {
	class BufferObject;
}
class TextureManager;

struct MaterialUniforms {
};

struct MaterialOptions {
	std::shared_ptr<MaterialUniforms> uniforms;
	std::array<u16, 4> texture_ids;
};

struct Material {
	gl::Shader vertex_shader, geometry_shader, fragment_shader;
	gl::ShaderProgram shader_program;
	size_t options_size;
	std::array<GLuint, 4> tex_uniform_index;

	Material()
		: vertex_shader(GL_VERTEX_SHADER),
		geometry_shader(GL_GEOMETRY_SHADER),
		fragment_shader(GL_FRAGMENT_SHADER)
	{
		tex_uniform_index.fill(0);
	}

	void setOptionsSize(size_t s) { options_size = s; }

	void loadFromFiles(const char* vert, const char* frag, const char* geom = nullptr);
};
