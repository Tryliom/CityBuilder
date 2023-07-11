#include "Grid.h"

Grid::Grid(int width, int height, int tileSize)
{
	Width = width;
	Height = height;
	TileSize = tileSize;

	for (int w = 0; w < width / tileSize; w++)
	{
		for (int h = 0; h < height / tileSize; h++)
		{
			Tiles.emplace_back(Vector2F { static_cast<float>(w) * tileSize, static_cast<float>(h) * tileSize}, tileSize);
		}
	}
}