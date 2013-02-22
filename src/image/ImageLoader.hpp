#ifndef BLOODY3D_IMAGE_IMAGELOADER_HPP
#define BLOODY3D_IMAGE_IMAGELOADER_HPP

#include "util/noncopyable.hpp"

#include <string>
#include <stdexcept>

namespace image {

class PngException : public std::runtime_error
{
public:
	PngException(const std::string& str);
};

class Image
{
	NONCOPYABLE(Image);
public:
	Image();
	Image(unsigned int width, unsigned int height);
	~Image();

	void initialize(unsigned char* data, unsigned int width, unsigned int height);
	void clear();

	unsigned int getWidth() const;
	unsigned int getHeight() const;
	unsigned char* getData() const;

	static void loadPNGFileRGBA8(Image& image, std::string& fname);

protected:
	unsigned int width;
	unsigned int height;
	unsigned char* data;
};

void preMultiplyAlpha(Image& image);

///////////////////////////////////////////////////////////////////////////////

inline PngException::PngException(const std::string& str)
	: std::runtime_error(str)
{
}

inline Image::Image()
	: width(~0u), height(~0u), data(0)
{
}

inline Image::Image(unsigned int width, unsigned int height)
	: width(~0u), height(~0u), data(0)
{
	initialize(nullptr, width, height);
}

inline Image::~Image()
{
	clear();
}

inline unsigned int Image::getWidth() const
{
	return width;
}

inline unsigned int Image::getHeight() const
{
	return height;
}

inline unsigned char* Image::getData() const
{
	return data;
}

}

#endif // BLOODY3D_IMAGE_IMAGELOADER_HPP
