#ifndef BLOODY3D_GL_VERTEXARRAYOBJECT
#define BLOODY3D_GL_VERTEXARRAYOBJECT

#include "../gl3w.hpp"

#include <boost/noncopyable.hpp>

namespace gl {

class VertexArrayObject : boost::noncopyable
{
public:
	VertexArrayObject();
	~VertexArrayObject();

	void bind() const;

	operator GLuint() const;

private:
	GLuint id;
};

inline VertexArrayObject::VertexArrayObject() 
{
	glGenVertexArrays(1, &id);
}

inline VertexArrayObject::~VertexArrayObject()
{
	glDeleteVertexArrays(1, &id);
}

inline void VertexArrayObject::bind() const
{
	glBindVertexArray(id);
}

inline VertexArrayObject::operator GLuint() const
{
	return id;
}

} // namespace gl

#endif // BLOODY3D_GL_VERTEXARRAYOBJECT
