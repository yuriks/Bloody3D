#pragma once

#include "GL3/gl3w.h"
#include "util/noncopyable.hpp"

namespace gl {

class Framebuffer
{
	NONCOPYABLE(Framebuffer);
public:
	Framebuffer();
	~Framebuffer();
	Framebuffer(Framebuffer&& o);

	void bind(GLenum target) const;

	operator GLuint() const;

private:
	GLuint id;
};

inline Framebuffer::Framebuffer()
{
	glGenFramebuffers(1, &id);
}

inline Framebuffer::~Framebuffer()
{
	if (id != 0)
		glDeleteFramebuffers(1, &id);
}

inline Framebuffer::Framebuffer(Framebuffer&& o)
	: id(o.id)
{
	o.id = 0;
}

inline void Framebuffer::bind(GLenum target) const
{
	glBindFramebuffer(target, id);
}

inline Framebuffer::operator GLuint() const
{
	return id;
}

} // namespace gl
