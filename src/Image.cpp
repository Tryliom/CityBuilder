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

void Image::AddImagesAtRow(const std::vector<Image>& images)
{
    // Add each image to a new row
    for (Image image : images)
    {
        int newWidth = _width;
        int newHeight = _height + image.GetHeight();
        uint8_t* bufferImage = image.GetBuffer();

        if (_width < image.GetWidth())
        {
            newWidth = image.GetWidth();
        }

        auto* newBuffer = (uint8_t*) malloc(newWidth * newHeight * Channels);

        memset(newBuffer, 0, newWidth * newHeight * Channels);

        // Copy the old buffer to the top of the image
        for (int y = 0; y < _height; y++)
        {
            for (int x = 0; x < _width; x++)
            {
                const int index = y * _width + x;
                const int newIndex = y * newWidth + x;

                for (int i = 0; i < Channels; i++)
                {
                    const int newChannelIndex = newIndex * Channels + i;
                    const int channelIndex = index * Channels + i;

                    if (newChannelIndex >= newWidth * newHeight * Channels)
                    {
                        LOG_ERROR("newChannelIndex >= newWidth * newHeight * Channels");
                    }

                    if (channelIndex >= _width * _height * Channels)
                    {
                        LOG_ERROR("channelIndex >= _width * _height * Channels");
                    }

                    newBuffer[newChannelIndex] = _buffer[channelIndex];
                }
            }
        }

        free(_buffer);

        // Copy the new image to the bottom of the image
        for (int y = 0; y < image.GetHeight(); y++)
        {
            for (int x = 0; x < image.GetWidth(); x++)
            {
                const int index = y * image.GetWidth() + x;
                const int newIndex = (y + _height) * newWidth + x;

                for (int i = 0; i < Channels; i++)
                {
                    newBuffer[newIndex * Channels + i] = bufferImage[index * Channels + i];
                }
            }
        }

        _width = newWidth;
        _height = newHeight;

        _buffer = newBuffer;

        free(bufferImage);
    }
}