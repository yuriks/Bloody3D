#include "GPUMesh.hpp"
#include "util/mmap.hpp"
#include "HWMesh.hpp"
#include <cassert>

GPUMesh GPUMeshTemplate::compile() {
	GPUMesh mesh;

	MMapHandle mmap_h = mmapFile(file.c_str());
	assert(mmap_h != -1);

	loadHWMesh(mmapGetData(mmap_h), fnv_hash_runtime(meshname.c_str()), mesh);

	mmapClose(mmap_h);

	mesh.material_id = material;

	return mesh;
}

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

void GPUMesh::loadIndices(const void* data, size_t data_size, size_t element_size) {
	ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
	indices_count = data_size / element_size;
	switch (element_size) {
		case 1: indices_type = GL_UNSIGNED_BYTE; break;
		case 2: indices_type = GL_UNSIGNED_SHORT; break;
		case 4: indices_type = GL_UNSIGNED_INT; break;
		default: assert(false);
	}
}
