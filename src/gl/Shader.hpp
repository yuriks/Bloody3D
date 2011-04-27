#ifndef BLOODY3D_GL_SHADER
#define BLOODY3D_GL_SHADER

#include "../gl3w.hpp"

#include <iosfwd>
#include <vector>
#include <boost/noncopyable.hpp>

namespace gl {

class Shader : boost::noncopyable
{
public:
	explicit Shader(GLenum type);
	~Shader();

	void setSource(const char* src) const;
	void setSource(std::ifstream& f) const;
	void compile() const;

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

inline Shader::operator GLuint() const
{
	return id;
}

} // namespace gl

#endif // BLOODY3D_GL_SHADER
