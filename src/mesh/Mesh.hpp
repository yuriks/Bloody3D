#ifndef BLOODY3D_MESH_MESH_HPP
#define BLOODY3D_MESH_MESH_HPP

#include "math/Vector.hpp"
#include "util/AlignedVector.hpp"
#include <vector>
#include <string>

struct Vertex
{
	math::vec3 position;
	math::vec3 normal;
	float tex_coord[2];
};

struct SubMesh
{
	util::AlignedVector<Vertex> vertices;
	std::vector<unsigned short> indices;
	std::string name;
	std::string material;
};

struct Mesh
{
	std::vector<SubMesh> sub_meshes;
};

#endif // BLOODY3D_MESH_MESH_HPP
