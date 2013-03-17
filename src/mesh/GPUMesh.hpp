#pragma once

#include "Heatwave.hpp"
#include "gl/VertexArrayObject.hpp"
#include "gl/BufferObject.hpp"
#include "Material.hpp"
#include "VertexFormats.hpp"
#include "util/Handle.hpp"

struct GPUMesh {
	GPUMesh() : vertex_format(vertex_fmt::FMT_NONE) {}
	GPUMesh(GPUMesh&& o)
		: vao(std::move(o.vao)), vbo(std::move(o.vbo)), ibo(std::move(o.ibo)),
		vertex_format(o.vertex_format), material_id(o.material_id),
		indices_count(o.indices_count), indices_type(o.indices_type),
		material_options(std::move(o.material_options))
	{}

	void loadVertexData(const void* data, size_t data_size, vertex_fmt::VertexFormat format);
	void loadIndices(const void* data, size_t data_size, size_t element_size);

	gl::VertexArrayObject vao;

	gl::BufferObject vbo;
	gl::BufferObject ibo;

	vertex_fmt::VertexFormat vertex_format;
	Handle material_id;
	unsigned int indices_count;
	GLenum indices_type;
	MaterialOptions material_options;
};
