#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#if defined(_WIN32)
    #include "malloc.h"
#endif

#include "Logger.h"

#include <string>

const int Channels = 4;

Image::Image()
{
    _width = 0;
    _height = 0;

    _buffer = nullptr;
}

Image::Image(int width, int height, uint8_t* buffer)
{
    _width = width;
    _height = height;

    _buffer = (uint8_t*) malloc(width * height * Channels);

    memcpy(_buffer, buffer, width * height * Channels);
}

Image::Image(const char* filename)
{
	int width, height, channels;

	stbi_uc* pixels = stbi_load(filename, &width, &height, &channels, 4);

	if (pixels == nullptr)
	{
		LOG_ERROR("Failed to load image: " + std::string(filename));
		return;
	}

	_width = width;
	_height = height;

	_buffer = (uint8_t*) malloc(width * height * Channels);

	memcpy(_buffer, pixels, width * height * Channels);

	stbi_image_free(pixels);
}

Image Image::Cut(int x, int y, int width, int height)
{
    auto* buffer = (uint8_t*) malloc(width * height * Channels);

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            const int index = j * width + i;
            const int originalIndex = (j + y) * _width + (i + x);

            buffer[index] = _buffer[originalIndex];
        }
    }

    return { width, height, buffer };
}
