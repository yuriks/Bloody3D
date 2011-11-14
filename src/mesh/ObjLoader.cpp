#include "ObjLoader.hpp"

#include "math/Vector.hpp"
#include "util/AlignedVector.hpp"
#include <sstream>
#include <iostream>
#include <tuple>
#include <map>
#include <deque>

using math::vec2;
using math::vec3;
using std::get;

Mesh load_obj(std::istream& f)
{
	typedef std::tuple<unsigned int, unsigned int, unsigned int> Triangle;
	typedef std::tuple<float, float> floatPair;

	Mesh mesh;
	SubMesh* current_submesh = nullptr;

	util::AlignedVector<vec3> pos_db;
	util::AlignedVector<vec3> norm_db;
	std::vector<floatPair> uv_db;
	std::map<Triangle, unsigned short> vertex_cache;

	std::string line;
	while (std::getline(f, line))
	{
		int comment_start = line.find('#');
		if (comment_start != std::string::npos)
			line.erase(comment_start);

		std::istringstream ss(line);

		std::string command;
		if (!(ss >> command))
			continue;

		if (command == "o")
		{
			mesh.sub_meshes.resize(mesh.sub_meshes.size() + 1);
			current_submesh = &mesh.sub_meshes.back();
			vertex_cache.clear();

			ss >> current_submesh->name;
		}
		else if (command == "v")
		{
			float x, y, z;
			ss >> x >> y >> z;
			pos_db.push_back(vec3(x, y, z));
		}
		else if (command == "vt")
		{
			float u, v;
			ss >> u >> v;
			uv_db.push_back(floatPair(u, v));
		}
		else if (command == "vn")
		{
			float x, y, z;
			ss >> x >> y >> z;
			norm_db.push_back(vec3(x, y, z));
		}
		else if (command == "f")
		{
			if (current_submesh == nullptr)
				std::cerr << "'f' command before any 'o'" << std::endl;
			else
				for (unsigned int i = 0; i < 3; ++i)
				{
					std::string tri_str;
					ss >> tri_str;

					Triangle tri;

					if (tri_str[0] == '/') {
						get<0>(tri) = 0;
						tri_str.erase(0, 1);
					} else {
						size_t off;
						get<0>(tri) = std::stoi(tri_str, &off);
						tri_str.erase(0, off+1);
					}
					if (tri_str[0] == '/') {
						get<1>(tri) = 0;
						tri_str.erase(0, 1);
					} else {
						size_t off;
						get<1>(tri) = std::stoi(tri_str, &off);
						tri_str.erase(0, off+1);
					}
					if (tri_str.empty()) {
						get<2>(tri) = 0;
						tri_str.erase(0, 1);
					} else {
						size_t off;
						get<2>(tri) = std::stoi(tri_str, &off);
						tri_str.erase(0, off);
					}

					// Try to find key in map
					auto lb = vertex_cache.lower_bound(tri);
					if (lb != vertex_cache.end() && !(vertex_cache.key_comp()(tri, lb->first)))
					{
						// Already exists
						current_submesh->indices.push_back(lb->second);
					}
					else
					{
						// Doesn't exist yet
						vertex_fmt::Pos3f_Norm3f_Tex2f vert;
						unsigned int pos_i = get<0>(tri);
						unsigned int uv_i = get<1>(tri);
						unsigned int norm_i = get<2>(tri);

						if ((pos_i != 0 && pos_i-1 >= pos_db.size()) || (norm_i != 0 && norm_i-1 >= norm_db.size()) || (uv_i != 0 && uv_i-1 >= uv_db.size())) {
							std::cerr << "Invalid vertex index: " << pos_i << ' ' << uv_i << ' ' << norm_i << std::endl;
							goto abort_while;
						}
						vert.pos = pos_i == 0 ? vec3(0.f) : pos_db[pos_i-1];
						vert.norm = norm_i == 0 ? vec3(0.f) : norm_db[norm_i-1];
						auto pair = uv_i == 0 ? floatPair(0.f, 0.f) : uv_db[uv_i-1];
						vert.tex_coord[0] = get<0>(pair);
						vert.tex_coord[1] = get<1>(pair);

						int i = current_submesh->vertices.size();
						current_submesh->vertices.push_back(vert);
						current_submesh->indices.push_back(i);
						vertex_cache.insert(lb, std::make_pair(tri, i));
					}
				}
		}
		else if (command == "usemtl")
		{
			if (current_submesh == nullptr)
				std::cerr << "'usemtl' command before any 'o'" << std::endl;
			else
			{
				ss >> current_submesh->material;
			}
		}
		else if (command == "s")
		{
			std::string tmp;
			ss >> tmp;
			if (tmp != "off")
			{
				std::cerr << "Smoothing groups not supported!" << std::endl;
			}
		}
	}
abort_while:

	return mesh;
}
