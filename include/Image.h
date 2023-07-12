#pragma once

#include <cstdint>
#include <vector>

class Image
{
public:
    Image();
    Image(int width, int height, uint8_t* buffer);
	explicit Image(const char* filename);

private:
    int _width;
    int _height;

    uint8_t* _buffer;

public:
	[[nodiscard]] int GetWidth() const { return _width; }
	[[nodiscard]] int GetHeight() const { return _height; }

    [[nodiscard]] uint8_t* GetBuffer() const { return _buffer; }
    [[nodiscard]] size_t GetBufferSize() const { return _width * _height * 4; }

    Image Cut(int x, int y, int width, int height);

    void AddImagesAtRow(const std::vector<Image>& images);
};