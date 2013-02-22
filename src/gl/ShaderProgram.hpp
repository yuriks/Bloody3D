#ifndef BLOODY3D_GL_SHADERPROGRAM
#define BLOODY3D_GL_SHADERPROGRAM

#include "Shader.hpp"

#include "GL3/gl3w.h"

#include <iostream>
#include <vector>
#include "util/noncopyable.hpp"

namespace gl {

class ShaderProgram
{
	NONCOPYABLE(ShaderProgram);
public:
	ShaderProgram();
	ShaderProgram(ShaderProgram&& o) : id(o.id) {
		o.id = 0;
	}
	~ShaderProgram();

	void use() const;
	void bindAttribute(GLuint location, const GLchar* name) const;
	GLuint getUniformLocation(const GLchar* name) const;

	void attachShader(const Shader& shader) const;
	void link() const;

	bool linkSuccess() const;
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
	if (id != 0)
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

inline bool ShaderProgram::linkSuccess() const {
	GLint result;
	glGetProgramiv(id, GL_LINK_STATUS, &result);
	return result == GL_TRUE;
}

inline ShaderProgram::operator GLuint() const
{
	return id;
}

} // namespace gl

#endif // BLOODY3D_GL_SHADERPROGRAM
