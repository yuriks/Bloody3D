#ifndef BLOODY3D_GL_BUFFEROBJECT
#define BLOODY3D_GL_BUFFEROBJECT

#include "GL3/gl3w.h"
#include "util/noncopyable.hpp"

namespace gl {

class BufferObject
{
	NONCOPYABLE(BufferObject);
public:
	BufferObject();
	~BufferObject();
	BufferObject(BufferObject&& o);

	void bind(GLenum target) const;

	operator GLuint() const;

private:
	GLuint id;
};

inline BufferObject::BufferObject()
{
	glGenBuffers(1, &id);
}

inline BufferObject::~BufferObject()
{
	if (id != 0)
		glDeleteBuffers(1, &id);
}

inline BufferObject::BufferObject(BufferObject&& o)
	: id(o.id)
{
	o.id = 0;
}

inline void BufferObject::bind(GLenum target) const
{
	glBindBuffer(target, id);
}

inline BufferObject::operator GLuint() const
{
	return id;
}

} // namespace gl

#endif // BLOODY3D_GL_BUFFEROBJECT
