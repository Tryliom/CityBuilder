#pragma once

#include <utility>
#include <vector>

#include "Tile.h"
#include "Maths.h"

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
    void Update();

    Vector2I GetTilePosition(Vector2F position) const;
    void SetTile(Vector2I position, Tile tile);
    void RemoveTile(Vector2I position);
};