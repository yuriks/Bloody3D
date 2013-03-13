#ifndef BLOODY3D_GL_SHADER
#define BLOODY3D_GL_SHADER

#include <GL/gl3w.h>

#include <iosfwd>
#include <vector>
#include "util/noncopyable.hpp"

namespace gl {

class Shader
{
	NONCOPYABLE(Shader);
public:
	explicit Shader(GLenum type);
	Shader(Shader&& o) : id(o.id) {
		o.id = 0;
	}
	~Shader();

	void setSource(const char* src) const;
	void setSource(std::ifstream& f) const;
	void compile() const;

	bool compileSuccess() const;
	void printInfoLog(std::ostream& s) const;

	operator GLuint() const;

private:
	GLuint id;
};

inline Shader::Shader(GLenum type)
{
	id = glCreateShader(type);
}

inline Shader::~Shader()
{
	if (id != 0)
		glDeleteShader(id);
}

inline void Shader::setSource(const char* src) const
{
	glShaderSource(id, 1, &src, 0);
}

inline void Shader::compile() const
{
	glCompileShader(id);
}

inline bool Shader::compileSuccess() const {
	GLint result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	return result == GL_TRUE;
}

inline Shader::operator GLuint() const
{
	return id;
}

} // namespace gl

#endif // BLOODY3D_GL_SHADER
