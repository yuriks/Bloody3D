#pragma once

#include "gl/Texture.hpp"
#include "Heatwave.hpp"
#include <map>
#include <string>
#include "util/ObjectPool.hpp"

enum TexFlags {
	TEXF_NONE = 0,
	TEXF_ALPHA = 1,
	TEXF_SRGB = 2
};

class TextureManager {
public:
	ObjectPool<gl::Texture> textures;
	std::map<std::string, Handle> fname_map;

	Handle getTexture(const std::string& fname, TexFlags flags = TEXF_NONE);
};
