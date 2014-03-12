#pragma once

#include "Heatwave.hpp"
#include "VertexFormats.hpp"

struct GPUMesh;

struct HWMeshHeader {
	static const int HWMESH_VERSION = 0;

	// Header magic
	char magic[6]; // = "HWMESH"
	// Version number,
	u16 version; // = HWMESH_VERSION

	u16 num_meshes;
};
static_assert(sizeof(HWMeshHeader) == 10, "Struct size changed! (Breaks I/O compatibility.)");

struct HWMeshIndex {
	u32 name_hash; // Hashed string identifier for mesh
	u32 file_offset; // Offset into file where data begins
};
static_assert(sizeof(HWMeshIndex) == 8, "Struct size changed! (Breaks I/O compatibility.)");

struct HWMeshData {
	static const int MAX_SUBMESHES = 16;

	u8 index_type; // 1 = UNSIGNED_BYTE, 2 = UNSIGNED_SHORT
	u8 vertex_format; // See VertexFormat enum in VertexFormats.hpp
	u8 num_submeshes; // max MAX_SUBMESHES

	u8 reserved;

	u16 submesh_indices_offset[MAX_SUBMESHES];
	u16 submesh_indices_size[MAX_SUBMESHES];

	u32 vertex_data_size; // in bytes
	u32 index_data_size; // in bytes

	//ALIGN(16);
	//u8 vertex_data[vertex_data_size];

	//ALIGN(4);
	//u8 index_data[index_data_size];
};
static_assert(sizeof(HWMeshData) == 76, "Struct size changed! (Breaks I/O compatibility.)");

void loadHWMesh(const void* file_data_ptr, u32 name_hash, GPUMesh& gpu_mesh);
