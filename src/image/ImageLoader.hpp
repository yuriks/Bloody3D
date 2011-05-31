#ifndef BLOODY3D_IMAGE_IMAGELOADER_HPP
#define BLOODY3D_IMAGE_IMAGELOADER_HPP

#include <boost/noncopyable.hpp>

#include <iosfwd>
#include <stdexcept>

namespace image {

class PngException : public std::runtime_error
{
public:
	PngException(const std::string& str);
};

class Image : boost::noncopyable
{
public:
	Image();
	Image(unsigned int width, unsigned int height);
	~Image();

	void initialize(unsigned int width, unsigned int height);
	void clear();

	unsigned int getWidth() const;
	unsigned int getHeight() const;
	unsigned char* getData() const;

	static void loadPNGFileRGBA8(Image& image, std::istream& stream);

private:
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
	initialize(width, height);
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
