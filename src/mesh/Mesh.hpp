#ifndef BLOODY3D_MESH_MESH_HPP
#define BLOODY3D_MESH_MESH_HPP

#include "Vector.hpp"
#include <vector>
#include <string>

struct Vertex
{
	math::vec3 position;
	math::vec3 normal;
	//math::vec2 tex_coord;
};

struct SubMesh
{
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
	std::string name;
	std::string material;
};

struct Mesh
{
	std::vector<SubMesh> sub_meshes;
};

#endif // BLOODY3D_MESH_MESH_HPP
