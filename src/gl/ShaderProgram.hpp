#ifndef BLOODY3D_GL_SHADERPROGRAM
#define BLOODY3D_GL_SHADERPROGRAM

#include "Shader.hpp"

#include "../gl3w.hpp"

#include <iostream>
#include <vector>
#include <boost/noncopyable.hpp>

namespace gl {

class ShaderProgram : boost::noncopyable
{
public:
	ShaderProgram();
	~ShaderProgram();

	void use() const;
	void bindAttribute(GLuint location, const GLchar* name) const;
	GLuint getUniformLocation(const GLchar* name) const;

	void attachShader(const Shader& shader) const;
	void link() const;

	void printInfoLog(std::ostream& s) const;

	operator GLuint() const;

private:
	GLuint id;
};

inline ShaderProgram::ShaderProgram()
{
	id = glCreateProgram();
}

inline ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(id);
}

inline void ShaderProgram::use() const
{
	glUseProgram(id);
}

inline void ShaderProgram::bindAttribute(GLuint location, const GLchar* name) const
{
	glBindAttribLocation(id, location, name);
}

inline GLuint ShaderProgram::getUniformLocation(const GLchar* name) const
{
	return glGetUniformLocation(id, name);
}

inline void ShaderProgram::attachShader(const Shader& shader) const
{
	glAttachShader(id, shader);
}

inline void ShaderProgram::link() const
{
	glLinkProgram(id);
}

inline ShaderProgram::operator GLuint() const
{
	return id;
}

} // namespace gl

#endif // BLOODY3D_GL_SHADERPROGRAM
