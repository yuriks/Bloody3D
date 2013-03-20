#pragma once

#include "gl/Texture.hpp"
#include "Heatwave.hpp"
#include <string>

enum TexFlags {
	TEXF_NONE  = 0,
	TEXF_ALPHA = BIT(0),
	TEXF_SRGB  = BIT(1)
};

struct Texture {
	gl::Texture gl_tex;

	Texture(gl::Texture&& gl_tex)
		: gl_tex(std::move(gl_tex))
	{}

	Texture(Texture&& o)
		: gl_tex(std::move(o.gl_tex))
	{}
};

struct TextureTemplate {
	std::string file;
	TexFlags flags;

	Texture compile();
};
