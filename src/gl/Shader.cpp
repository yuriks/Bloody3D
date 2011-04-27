#include "Shader.hpp"
#include "ShaderProgram.hpp"

#include <iostream>
#include <fstream>

namespace gl
{

void Shader::setSource(std::ifstream& f) const
{
	std::vector<char> str;

	f.seekg(0, std::ios::end);
	str.reserve((unsigned int)f.tellg() + 1);
	f.seekg(0);

	str.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
	str.push_back('\0');

	setSource(&str[0]);
}

void Shader::printInfoLog(std::ostream& s) const
{
	GLint log_size;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_size);

	std::vector<char> log_buf(log_size);
	glGetShaderInfoLog(id, log_buf.size(), 0, &log_buf[0]);

	s << &log_buf[0];
}

void ShaderProgram::printInfoLog(std::ostream& s) const
{
	GLint log_size;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_size);

	std::vector<char> log_buf(log_size);
	glGetProgramInfoLog(id, log_buf.size(), 0, &log_buf[0]);

	s << &log_buf[0];
}

} // namespace gl
