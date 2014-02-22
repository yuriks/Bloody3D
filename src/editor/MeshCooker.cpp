#include "MeshCooker.hpp"

#include "mesh/ObjLoader.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/VertexFormats.hpp"
#include "mesh/HWMesh.hpp"
#include "util/StringHash.hpp"
#include "util/AlignedVector.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cassert>

namespace editor {

namespace {

static vertex_fmt::VertexFormat parse_format(const char* fmt) {
	if (std::strcmp(fmt, "Pos3f") == 0) {
		return vertex_fmt::FMT_POS3F;
	} else if (std::strcmp(fmt, "Pos3f_Norm3f_Tex2f") == 0) {
		return vertex_fmt::FMT_POS3F_NORM3F_TEX2F;
	} else {
		return vertex_fmt::FMT_NONE; // Invalid value
	}
}

static const size_t format_sizes[] = {
	0,
	sizeof(vertex_fmt::Pos3f),
	sizeof(vertex_fmt::Pos3f_Norm3f_Tex2f)
};

} // namespace

bool cook_mesh(int& argc, char **& argv) {
	if (argc < 1) {
		std::cerr << "[CookMesh] Missing output file." << std::endl;
		return false;
	}

	//std::ofstream out_file(argv[0], std::ios::out | std::ios::binary);
	const char *out_fname = argv[0];
	argc -= 1; argv += 1;

	std::vector<mesh::HWMeshIndex> index;
	std::vector<mesh::HWMeshData> mesh_data;
	std::vector<const char*> input_filenames;

	while (argc > 0) {
		if (argc < 3) {
			std::cerr << "[CookMesh] Missing mesh parameters: in_file mesh_name format" << std::endl;
			return false;
		}

		input_filenames.push_back(argv[0]);

		{
			mesh::HWMeshIndex i;
			i.name_hash = util::fnv_hash_runtime(argv[1]);
			i.file_offset = ~0u;

			index.push_back(i);
		}

		{
			vertex_fmt::VertexFormat fmt = parse_format(argv[2]);
			if (fmt == vertex_fmt::FMT_NONE) {
				std::cerr << "[CookMesh] Unknown vertex format: " << argv[2] << std::endl;
				return false;
			}

			mesh::HWMeshData d;
			d.vertex_format = fmt;
			d.reserved = 0;

			mesh_data.push_back(d);
		}

		argc -= 3; argv += 3;
	}

	std::ofstream out_file(out_fname, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out_file) {
		std::cerr << "[CookMesh] Can't open output file '" << out_fname << "'" << std::endl;
		return false;
	}

	mesh::HWMeshHeader header;
	std::copy_n("HWMESH", 6, header.magic);
	header.version = 0;
	header.num_meshes = input_filenames.size();

	out_file.write(reinterpret_cast<char*>(&header), sizeof(mesh::HWMeshHeader));

	std::streamoff index_offset = out_file.tellp();
	out_file.write(reinterpret_cast<char*>(index.data()), sizeof(mesh::HWMeshIndex) * index.size());

	for (unsigned int i = 0; i < input_filenames.size(); ++i) {
		std::ifstream objf(input_filenames[i]);
		if (!objf) {
			std::cerr << "[CookMesh] Can't open input file '" << input_filenames[i] << "'" << std::endl;
			return false;
		}

		Mesh obj_mesh = load_obj(objf);
		for (unsigned int j = 0; j < obj_mesh.sub_meshes.size(); ++j) {
			SubMesh& m = obj_mesh.sub_meshes[j];
			std::cerr << "[CookMesh] " << m.name << ": verts(" << m.vertices.size() <<
				") indices(" << m.indices.size() << ")\n";
		}

		mesh_data[i].num_submeshes = obj_mesh.sub_meshes.size();

		unsigned int max_index = 0;
		for (unsigned int j = 0; j < obj_mesh.sub_meshes.size(); ++j) {
			mesh_data[i].submesh_indices_offset[j] = max_index;
			mesh_data[i].submesh_indices_size[j] = obj_mesh.sub_meshes[j].indices.size();
			max_index += obj_mesh.sub_meshes[j].vertices.size();
		}
		for (unsigned int j = obj_mesh.sub_meshes.size(); j < 16; ++j) {
			mesh_data[i].submesh_indices_offset[j] = 0;
			mesh_data[i].submesh_indices_size[j] = 0;
		}

		if (max_index <= 256) {
			mesh_data[i].index_type = 1; // UNSIGNED_BYTE
		} else {
			mesh_data[i].index_type = 2; // UNSIGNED_SHORT
		}

		unsigned int num_indices = std::accumulate(std::begin(mesh_data[i].submesh_indices_size), std::end(mesh_data[i].submesh_indices_size), 0);
		mesh_data[i].index_data_size = num_indices * mesh_data[i].index_type;
		mesh_data[i].vertex_data_size =	max_index * format_sizes[mesh_data[i].vertex_format];

		index[i].file_offset = static_cast<u32>(out_file.tellp());

		out_file.write(reinterpret_cast<char*>(&mesh_data[i]), sizeof(mesh::HWMeshData));

		static const unsigned char alignment_padding[15] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
		{
			// Align to a 16-byte boundary
			std::streamoff cur_pos = out_file.tellp();
			std::streamoff aligned_pos = ((cur_pos - 1) | 0xf) + 1;
			out_file.write(reinterpret_cast<const char*>(alignment_padding), aligned_pos - cur_pos);
		}

		if (mesh_data[i].vertex_format == vertex_fmt::FMT_POS3F) {
			util::AlignedVector<vertex_fmt::Pos3f> v_data;
			v_data.resize(max_index);

			auto cur_output = std::begin(v_data);
			for (unsigned int j = 0; j < obj_mesh.sub_meshes.size(); ++j) {
				SubMesh& m = obj_mesh.sub_meshes[j];
				cur_output = std::transform(std::begin(m.vertices), std::end(m.vertices), cur_output,
					[&](const vertex_fmt::Pos3f_Norm3f_Tex2f& v) -> vertex_fmt::Pos3f {
						vertex_fmt::Pos3f nv;
						nv.pos = v.pos;
						return nv;
					}
				);
			}

			assert(v_data.size() * sizeof(vertex_fmt::Pos3f) == mesh_data[i].vertex_data_size);
			out_file.write(reinterpret_cast<char*>(v_data.data()), v_data.size() * sizeof(vertex_fmt::Pos3f));
		} else if (mesh_data[i].vertex_format == vertex_fmt::FMT_POS3F_NORM3F_TEX2F) {
			util::AlignedVector<vertex_fmt::Pos3f_Norm3f_Tex2f> v_data;
			v_data.resize(max_index);

			auto cur_output = std::begin(v_data);
			for (unsigned int j = 0; j < obj_mesh.sub_meshes.size(); ++j) {
				SubMesh& m = obj_mesh.sub_meshes[j];
				cur_output = std::copy(std::begin(m.vertices), std::end(m.vertices), cur_output);
			}

			assert(v_data.size() * sizeof(vertex_fmt::Pos3f_Norm3f_Tex2f) == mesh_data[i].vertex_data_size);
			out_file.write(reinterpret_cast<char*>(v_data.data()), v_data.size() * sizeof(vertex_fmt::Pos3f_Norm3f_Tex2f));
		}

		{
			// Align to a 4-byte boundary
			std::streamoff cur_pos = out_file.tellp();
			std::streamoff aligned_pos = ((cur_pos - 1) | 0x3) + 1;
			out_file.write(reinterpret_cast<const char*>(alignment_padding), aligned_pos - cur_pos);
		}

		// TODO: Refactor this into a template function or something
		if (mesh_data[i].index_type == 1) {
			// UNSIGNED_BYTE
			std::vector<u8> i_data(num_indices);
			u8 cur_index_base = 0;
			auto cur_output = std::begin(i_data);
			for (unsigned int j = 0; j < obj_mesh.sub_meshes.size(); ++j) {
				SubMesh& m = obj_mesh.sub_meshes[j];
				cur_output = std::transform(std::begin(m.indices), std::end(m.indices), cur_output, [&](const u8& x) { return x + cur_index_base; });
				cur_index_base += m.indices.size();
			}

			assert(i_data.size() == mesh_data[i].index_data_size);
			out_file.write(reinterpret_cast<char*>(i_data.data()), i_data.size());
		} else {
			// UNSIGNED_SHORT
			std::vector<u16> i_data(num_indices);
			u16 cur_index_base = 0;
			auto cur_output = std::begin(i_data);
			for (unsigned int j = 0; j < obj_mesh.sub_meshes.size(); ++j) {
				SubMesh& m = obj_mesh.sub_meshes[j];
				cur_output = std::transform(std::begin(m.indices), std::end(m.indices), cur_output, [&](const u16& x) { return x + cur_index_base; });
				cur_index_base += m.indices.size();
			}

			assert(i_data.size() * sizeof(u16) == mesh_data[i].index_data_size);
			out_file.write(reinterpret_cast<char*>(i_data.data()), i_data.size() * sizeof(u16));
		}
	}

	// Update index with mesh offsets.
	out_file.seekp(index_offset);
	out_file.write(reinterpret_cast<char*>(index.data()), sizeof(mesh::HWMeshIndex) * index.size());

	return true;
}

} // namespace editor
