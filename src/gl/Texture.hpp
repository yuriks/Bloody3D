#ifndef BLOODY3D_GL_TEXTURE
#define BLOODY3D_GL_TEXTURE

#include "../gl3w.hpp"

#include <boost/noncopyable.hpp>

namespace gl {

class Texture : boost::noncopyable
{
public:
	Texture();
	~Texture();

	void bind(GLenum target) const;

	operator GLuint() const;

	// Provided for convenience, not automatically updated. (TODO: Probably remove.)
	int width, height;

private:
	GLuint id;
};

inline Texture::Texture()
	: width(-1), height(-1)
{
	glGenTextures(1, &id);
}

inline Texture::~Texture()
{
	glDeleteTextures(1, &id);
}

inline void Texture::bind(GLenum target) const
{
	glBindTexture(target, id);
}

inline Texture::operator GLuint() const
{
	return id;
}

} // namespace gl

#endif // BLOODY3D_GL_TEXTURE
