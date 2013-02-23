#ifndef BLOODY3D_MESH_MESH_HPP
#define BLOODY3D_MESH_MESH_HPP

#include "util/AlignedVector.hpp"
#include <vector>
#include <string>
#include "VertexFormats.hpp"

struct SubMesh
{
	util::AlignedVector<vertex_fmt::Pos3f_Norm3f_Tex2f> vertices;
	std::vector<unsigned short> indices;
	std::string name;
	std::string material;
};

struct Mesh
{
	std::vector<SubMesh> sub_meshes;
};

#endif // BLOODY3D_MESH_MESH_HPP
