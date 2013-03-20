#pragma once

#include "gl/Shader.hpp"
#include "gl/ShaderProgram.hpp"
#include "Heatwave.hpp"
#include <GL/gl3w.h>
#include <memory>
#include <array>
#include <string>
#include "util/Handle.hpp"
#include "util/StringHash.hpp"

namespace gl {
	class BufferObject;
}
class TextureManager;

struct MaterialUniforms {
};

struct MaterialOptions {
	std::shared_ptr<MaterialUniforms> uniforms;
	std::array<Handle, 4> texture_ids;

	template <typename T>
	void reflect(T& f) {
		f(texture_ids[0], HASHSTR("tex0"));
		f(texture_ids[1], HASHSTR("tex1"));
		f(texture_ids[2], HASHSTR("tex2"));
		f(texture_ids[3], HASHSTR("tex3"));
	}
};

struct Material {
	gl::ShaderProgram shader_program;
	size_t options_size;

	Material(gl::ShaderProgram&& shader_program, size_t options_size)
		: shader_program(std::move(shader_program)),
		options_size(options_size)
	{}

	Material(Material&& o)
		: shader_program(std::move(o.shader_program)),
		options_size(o.options_size)
	{}
};

struct MaterialTemplate {
	size_t options_size;

	void attachShader(const std::string& fname, GLenum type);

	void attachShaders(const std::string& vert, const std::string& frag) {
		attachShader(vert, GL_VERTEX_SHADER);
		attachShader(frag, GL_FRAGMENT_SHADER);
	}

	void attachShaders(const std::string& base_fname) {
		attachShaders(base_fname + ".vert", base_fname + ".frag");
	}

	Material compile();
	void clear();

private:
	std::vector<gl::Shader> shaders;
};
