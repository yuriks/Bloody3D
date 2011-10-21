#pragma once

#include "gl/Texture.hpp"
#include "Heatwave.hpp"
#include <boost/noncopyable.hpp>
#include <map>
#include <vector>
#include <string>

class TextureManager : boost::noncopyable {
public:
	u16 loadTexture(const char* fname);
	gl::Texture* lookupTexture(u16 tex_id);

private:
	std::map<std::string, u16> fname_map;
	std::vector<gl::Texture> texture_map;
};