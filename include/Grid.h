#pragma once

#include <utility>
#include <vector>

#include "Tile.h"

class Grid
{
public:
	Grid(int width, int height, int tileSize);

	int Width;
	int Height;
	int TileSize;

	std::vector<Tile> Tiles = std::vector<Tile>();

};