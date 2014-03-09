#include "ImageLoader.hpp"

#include <cassert>
#include <cstdint>
#include <istream>

#include "stb_image.h"
#include <csetjmp>

void Image::initialize(unsigned char* data_, unsigned int width_, unsigned int height_)
{
	assert(width_ != ~0u);
	assert(height_ != ~0u);

	clear();
	data = data_;
	width = width_;
	height = height_;
}

void Image::clear()
{
	stbi_image_free(data);
	data = nullptr;
	width = ~0u;
	height = ~0u;
}

void Image::loadPNGFileRGBA8(Image& image, const std::string& filename)
{
	int width, height;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, nullptr, 4);

	if (data == nullptr) {
		throw PngException(stbi_failure_reason());
	}
	image.initialize(data, width, height);
}

bool isBigEndian()
{
	union
	{
		char c[2];
		uint16_t i;
	} foo;
	foo.i = 0x1020;
	return foo.c[0] == 0x10;
}

void preMultiplyAlpha(Image& image)
{
	unsigned char* data = image.getData();

	const unsigned int size = image.getWidth() * image.getHeight() * 4;

	for (unsigned int i = 0; i < size; i += 4) {
		unsigned int alpha = data[i+3];
		data[i+0] = data[i+0] * alpha / 255;
		data[i+1] = data[i+1] * alpha / 255;
		data[i+2] = data[i+2] * alpha / 255;
	}
}
