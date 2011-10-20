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
					// Replace all '/'s with ' '
					for (unsigned int p = tri_str.find('/'); p != std::string::npos; p = tri_str.find('/', p + 1))
					{
						tri_str[p] = ' ';
					}

					std::istringstream fss(tri_str);
					Triangle tri;
					fss >> get<0>(tri) >> get<1>(tri) >> get<2>(tri);

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
						unsigned int pos_i = get<0>(tri)-1;
						unsigned int uv_i = get<1>(tri)-1;
						unsigned int norm_i = get<2>(tri)-1;

						if (pos_i >= pos_db.size() || norm_i >= norm_db.size() || uv_i >= uv_db.size()) {
							std::cerr << "Invalid vertex index: " << pos_i << ' ' << uv_i << ' ' << norm_i << std::endl;
							goto abort_while;
						}
						vert.pos = pos_db[pos_i];
						vert.norm = norm_db[norm_i];
						auto pair = uv_db[uv_i];
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
