#include "gl/Shader.h"
#include "gl/ShaderProgram.h"

namespace gl
{

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
