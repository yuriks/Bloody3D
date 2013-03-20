#pragma once

#include "gl/Texture.hpp"
#include "Heatwave.hpp"
#include <string>
#include "util/StringHash.hpp"

enum TexFlags : u8 {
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
	u8 flags;

	Texture compile();

	TextureTemplate()
		: flags(TEXF_NONE)
	{}

	template <typename T>
	void reflect(T& f) {
		f(file, HASHSTR("file"));
		f.flags(flags, HASHSTR("flags"))
			.flag(TEXF_ALPHA, HASHSTR("alpha"))
			.flag(TEXF_SRGB, HASHSTR("srgb"));
	}
};
