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
		R = ((color >> 24) & 0xFF) / 255.f;
		G = ((color >> 16) & 0xFF) / 255.f;
		B = ((color >> 8) & 0xFF) / 255.f;
		A = (color & 0xFF) / 255.f;
	}

	float R, G, B, A;
};