#include "Material.hpp"

#include "gl/BufferObject.hpp"
#include <string>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <regex>

static const std::regex include_re("#include\\s+\"(.*)\"\\s*");

static void preprocessFile(std::string& source, const std::string& fname, std::vector<std::string>& file_list) {
	static const std::string shader_path("data/shaders/");
	std::ifstream f(shader_path + fname);
	if (!f) {
		source.append("#error File not found: ");
		source.append(fname);
		source.append(".\n");
	}

	unsigned int file_n = file_list.size();
	file_list.push_back(fname);

	source.append("#line 1 ");
	source.append(std::to_string((unsigned long long)file_n));
	source.push_back('\n');

	std::string line;
	unsigned int line_n = 1;
	while (std::getline(f, line)) {
		line_n += 1;
		if (line.substr(0, 8) == "#include") {
			std::smatch results;
			if (std::regex_match(line, results, include_re)) {
				preprocessFile(source, results[1].str(), file_list);
				
				source.append("#line ");
				source.append(std::to_string((unsigned long long)line_n));
				source.push_back(' ');
				source.append(std::to_string((unsigned long long)file_n));
				source.push_back('\n');
			} else {
				source.append("#error Malformed include directive.\n");
			}
		} else {
			source.append(line);
			source.push_back('\n');
		}
	}
}

static void loadShader(const char* fname, gl::Shader& shader) {
	std::vector<std::string> filenames;
	std::string source;

	preprocessFile(source, fname, filenames);
	shader.setSource(source.c_str());
	shader.compile();
	if (!shader.compileSuccess()) {
		for (unsigned int i = 0; i < filenames.size(); ++i) {
			std::cerr << '(' << i << "): " << filenames[i] << '\n';
		}
		shader.printInfoLog(std::cerr);
	}
}

void Material::loadFromFiles(const char* vert, const char* frag, const char* geom) {
	loadShader(vert, vertex_shader);
	loadShader(frag, fragment_shader);
	if (geom != nullptr)
		loadShader(geom, geometry_shader);

	shader_program.attachShader(vertex_shader);
	shader_program.attachShader(fragment_shader);
	if (geom != nullptr)
		shader_program.attachShader(geometry_shader);

	shader_program.link();
	if (!shader_program.linkSuccess()) {
		shader_program.printInfoLog(std::cerr);
	}

	GLuint sys_uniform_index = glGetUniformBlockIndex(shader_program, "SystemUniforms");
	glUniformBlockBinding(shader_program, sys_uniform_index, 0);
	GLuint mtl_uniform_index = glGetUniformBlockIndex(shader_program, "MaterialUniforms");
	glUniformBlockBinding(shader_program, mtl_uniform_index, 1);

	GLuint tex_uniform_index = glGetUniformLocation(shader_program, "tex");

	shader_program.use();
	if (tex_uniform_index != -1) {
		static const int samplers[4] = { 0, 1, 2, 3 };
		glUniform1iv(tex_uniform_index, 4, samplers);
	}
}
