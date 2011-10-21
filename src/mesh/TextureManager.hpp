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
	const gl::Texture* lookupTexture(u16 tex_id) const;

private:
	std::map<std::string, u16> fname_map;
	std::vector<gl::Texture> texture_map;
};