#include "ObjLoader.hpp"

#include "Vector.hpp"
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

	Mesh mesh;
	SubMesh* current_submesh = nullptr;

	std::deque<vec3> pos_db;
	std::deque<vec2> uv_db;
	std::deque<vec3> norm_db;
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
			vec3 v;
			ss >> v[0] >> v[2] >> v[1];
			pos_db.push_back(v);
		}
		else if (command == "vt")
		{
			vec2 uv;
			ss >> uv[0] >> uv[1];
			uv_db.push_back(uv);
		}
		else if (command == "vn")
		{
			vec3 norm;
			ss >> norm[0] >> norm[2] >> norm[1];
			norm_db.push_back(norm);
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
						Vertex vert;
						vert.position = pos_db.at(get<0>(tri)-1);
						vert.tex_coord = uv_db.at(get<1>(tri)-1);
						vert.normal = norm_db.at(get<2>(tri)-1);

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

	return mesh;
}