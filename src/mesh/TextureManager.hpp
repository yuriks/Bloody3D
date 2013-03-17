#pragma once

#include "gl/Texture.hpp"
#include "Heatwave.hpp"
#include "util/noncopyable.hpp"
#include <map>
#include <vector>
#include <string>
#include "util/ObjectPool.hpp"

enum TexFlags {
	TEXF_NONE = 0,
	TEXF_ALPHA = 1,
	TEXF_SRGB = 2
};

class TextureManager {
	NONCOPYABLE(TextureManager);
public:
	TextureManager() {}

	Handle loadTexture(const char* fname, TexFlags flags = TEXF_NONE);
	const gl::Texture* lookupTexture(Handle tex_id) const;

private:
	std::map<std::string, Handle> fname_map;
	ObjectPool<gl::Texture> textures;
};
