#include "TextureManager.hpp"

#include "image/ImageLoader.hpp"
#include <fstream>

Handle TextureManager::loadTexture(const char* fname, TexFlags flags) {
	static const std::string texture_path("data/textures/");

	auto lb = fname_map.lower_bound(fname);
	if (lb != fname_map.end() && !(fname_map.key_comp()(fname, lb->first)) && textures.isValid(lb->second)) {
		// Already exists
		return lb->second;
	} else {
		// Doesn't exist yet
		image::Image img;
		image::Image::loadPNGFileRGBA8(img, texture_path + fname);

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

		const Handle new_id = textures.insert(std::move(tex));
		fname_map.insert(lb, std::make_pair(fname, new_id));

		return new_id;
	}
}

const gl::Texture* TextureManager::lookupTexture(Handle tex_id) const {
	return textures[tex_id];
}
