#include "HWMesh.hpp"

#include "GPUMesh.hpp"
#include "Heatwave.hpp"

#include <cassert>

namespace mesh {

namespace {

template <typename T>
inline T* alignPtr(T* ptr, unsigned int align) {
	return (T*)((((std::intptr_t)ptr - 1) | (align-1)) + 1);
}

} // namespace

void loadHWMesh(const void* file_data_ptr, u32 name_hash, GPUMesh& gpu_mesh) {
	auto file_data = static_cast<const char*>(file_data_ptr);

	auto mesh_header = reinterpret_cast<const mesh::HWMeshHeader*>(file_data);
	assert(std::strcmp(mesh_header->magic, "HWMESH") == 0);
	assert(mesh_header->version == 0);

	auto mesh_index = reinterpret_cast<const mesh::HWMeshIndex*>(file_data + sizeof(mesh::HWMeshHeader));
	auto entry = std::find_if(mesh_index, mesh_index + mesh_header->num_meshes, [&](const mesh::HWMeshIndex& i) { return i.name_hash == name_hash; });
	assert(entry != mesh_index + mesh_header->num_meshes);

	auto mesh_data = reinterpret_cast<const mesh::HWMeshData*>(file_data + entry->file_offset);
	assert(mesh_data->vertex_format == vertex_fmt::FMT_POS3F_NORM3F_TEX2F);
	assert(mesh_data->num_submeshes == 1); // for now
	gpu_mesh.indices_count = mesh_data->submesh_indices_size[0];

	auto vertex_data = reinterpret_cast<const char*>(file_data + entry->file_offset + sizeof(mesh::HWMeshData));
	vertex_data = alignPtr(vertex_data, 16);

	auto index_data = vertex_data + mesh_data->vertex_data_size;
	index_data = alignPtr(index_data, 4);

	gpu_mesh.loadVertexData(vertex_data, mesh_data->vertex_data_size, (vertex_fmt::VertexFormat)mesh_data->vertex_format);
	gpu_mesh.loadIndices(index_data, mesh_data->index_data_size, mesh_data->index_type);
}

} // namespace mesh
