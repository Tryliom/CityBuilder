#pragma once

#include <cstdint>

struct Color
{
	Color(float r, float g, float b, float a)
	{
		R = r;
		G = g;
		B = b;
		A = a;
	}
	explicit Color(uint32_t color)
	{
		R = (float) ((color >> 24) & 0xFF) / 255.f;
		G = (float) ((color >> 16) & 0xFF) / 255.f;
		B = (float) ((color >> 8) & 0xFF) / 255.f;
		A = (float) (color & 0xFF) / 255.f;
	}

	float R, G, B, A;

	static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Purple;
    static const Color Yellow;
    static const Color Cyan;
    static const Color White;
};