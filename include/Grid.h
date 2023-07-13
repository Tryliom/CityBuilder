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

    Tile* _tiles;

public:
    void Draw();

    void SetTile(int x, int y, Tile tile);
    void RemoveTile(int x, int y);
};