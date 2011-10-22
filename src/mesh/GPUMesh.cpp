#include "GPUMesh.hpp"

using namespace vertex_fmt;

void GPUMesh::loadVertexData(const void* data, size_t data_size, vertex_fmt::VertexFormat format) {
	vertex_format = format;

	vao.bind();

	vbo.bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);

	switch (format) {
	case FMT_NONE:
		break;
	case FMT_POS3F:
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Pos3f), (void*)offsetof(Pos3f, pos));
		glEnableVertexAttribArray(0);
		break;
	case FMT_POS3F_NORM3F_TEX2F:
#define SIZEOFF(m) sizeof(Pos3f_Norm3f_Tex2f), (void*)offsetof(Pos3f_Norm3f_Tex2f, m)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, SIZEOFF(pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, SIZEOFF(norm));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, SIZEOFF(tex_coord));
		glEnableVertexAttribArray(2);
#undef SIZEOFF
		break;
	}
}

void GPUMesh::loadIndices(const u16* data, unsigned int count) {
	ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u16) * count, data, GL_STATIC_DRAW);
	indices_count = count;
}
