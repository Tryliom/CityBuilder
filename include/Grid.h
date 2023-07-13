#pragma once

#include <utility>
#include <vector>

#include "Tile.h"

class Grid
{
public:
	Grid(int width, int height, int tileSize);

private:
	int _width;
	int _height;
	int _tileSize;

public:
    std::vector<Tile> Tiles = std::vector<Tile>();

    void Draw();
};