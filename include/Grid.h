#pragma once

#include <utility>
#include <vector>

#include "Tile.h"
#include "Maths.h"

struct TilePosition
{
    int X;
    int Y;

    bool operator==(const TilePosition& other) const
    {
        return X == other.X && Y == other.Y;
    }
};

class Grid
{
public:
	Grid(int width, int height, int tileSize);

private:
	int _width;
	int _height;
	int _tileSize;

    Tile* _tiles;

    Texture getTexture(Tile& tile) const;

public:
    void Draw();
    void Update();

    [[nodiscard]] TilePosition GetTilePosition(Vector2F position) const;
    [[nodiscard]] Vector2F ToWorldPosition(TilePosition position) const;

    Tile& GetTile(TilePosition position);
    Tile& GetTile(int index);
    [[nodiscard]] int GetTileIndex(TilePosition position) const;

    void SetTile(TilePosition position, Tile tile);
    void RemoveTile(TilePosition position);

    std::vector<TilePosition> GetTiles(TileType type) const;
};