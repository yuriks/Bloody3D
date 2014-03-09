#include "Texture.hpp"

#include "ImageLoader.hpp"
#include <fstream>

static gl::Texture loadTexture(const std::string& fname, TexFlags flags) {
	Image img;
	Image::loadPNGFileRGBA8(img, fname);

	gl::Texture tex;
	tex.bind(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	tex.width = img.getWidth();
	tex.height = img.getHeight();

	static const GLenum texture_formats[] = {
		GL_RGB, /* TEXF_NONE */
		GL_RGBA, /* TEXF_ALPHA */
		GL_SRGB, /* TEXF_SRGB */
		GL_SRGB_ALPHA /* TEXF_ALPHA | TEXF_SRGB */
	};

	glTexImage2D(GL_TEXTURE_2D, 0, texture_formats[flags & (TEXF_ALPHA | TEXF_SRGB)], tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getData());
	glGenerateMipmap(GL_TEXTURE_2D);

	return tex;
}

Texture TextureTemplate::compile() {
	static const std::string texture_path("data/textures/");

	Image img;
	Image::loadPNGFileRGBA8(img, texture_path + file);

	gl::Texture tex;
	tex.bind(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	tex.width = img.getWidth();
	tex.height = img.getHeight();

	static const GLenum texture_formats[] = {
		GL_RGB, /* TEXF_NONE */
		GL_RGBA, /* TEXF_ALPHA */
		GL_SRGB, /* TEXF_SRGB */
		GL_SRGB_ALPHA /* TEXF_ALPHA | TEXF_SRGB */
	};

	glTexImage2D(GL_TEXTURE_2D, 0, texture_formats[flags & (TEXF_ALPHA | TEXF_SRGB)], tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getData());
	glGenerateMipmap(GL_TEXTURE_2D);

	return Texture(std::move(tex));
}
