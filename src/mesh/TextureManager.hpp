#pragma once

#include "gl/Texture.hpp"
#include "Heatwave.hpp"
#include <boost/noncopyable.hpp>
#include <map>
#include <vector>
#include <string>

enum TexFlags {
	TEXF_NONE = 0,
	TEXF_ALPHA = 1,
	TEXF_SRGB = 2
};

class TextureManager : boost::noncopyable {
public:
	u16 loadTexture(const char* fname, TexFlags flags = TEXF_NONE);
	const gl::Texture* lookupTexture(u16 tex_id) const;

private:
	std::map<std::string, u16> fname_map;
	std::vector<gl::Texture> texture_map;
};
