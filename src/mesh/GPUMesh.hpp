#pragma once

#include "Heatwave.hpp"
#include "gl/VertexArrayObject.hpp"
#include "gl/BufferObject.hpp"
#include "Material.hpp"
#include "VertexFormats.hpp"
#include <memory>

struct GPUMesh {
	GPUMesh() : vertex_format(vertex_fmt::FMT_NONE) {}

	void loadVertexData(const void* data, size_t data_size, vertex_fmt::VertexFormat format);
	void loadIndices(const u16* data, unsigned int count);

	gl::VertexArrayObject vao;

	gl::BufferObject vbo;
	gl::BufferObject ibo;

	vertex_fmt::VertexFormat vertex_format;
	int material_id;
	std::unique_ptr<MaterialOptions> material_options;
};
