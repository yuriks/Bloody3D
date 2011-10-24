#include "TextureManager.hpp"

#include "image/ImageLoader.hpp"
#include <fstream>

u16 TextureManager::loadTexture(const char* fname, TexFlags flags) {
	static const std::string texture_path("data/textures/");

	auto lb = fname_map.lower_bound(fname);
	if (lb != fname_map.end() && !(fname_map.key_comp()(fname, lb->first))) {
		// Already exists
		return lb->second;
	} else {
		// Doesn't exist yet
		image::Image img;
		std::ifstream f(texture_path + fname, std::ios::in | std::ios::binary);
		if (!f)
			return -1;
		image::Image::loadPNGFileRGBA8(img, f);

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

		u16 new_id = texture_map.size();
		fname_map.insert(lb, std::pair<std::string, u16>(fname, new_id));
		texture_map.push_back(std::move(tex));

		return new_id;
	}
}

const gl::Texture* TextureManager::lookupTexture(u16 tex_id) const {
	if (tex_id < texture_map.size()) {
		return &texture_map[tex_id];
	} else {
		return nullptr;
	}
}