#pragma once

#include <utility>
#include <vector>
#include <functional>

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

	TilePosition operator+(const TilePosition& other) const
	{
		return TilePosition{ X + other.X, Y + other.Y };
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

	// Texture
	static Texture getTreeTexture(Tile& tile);
	Texture getRoadTexture(TilePosition position);

public:
    void Draw();
    void Update();

    [[nodiscard]] TilePosition GetTilePosition(Vector2F position) const;
	[[nodiscard]] TilePosition GetTilePosition(int tileIndex) const;
    [[nodiscard]] Vector2F ToWorldPosition(TilePosition position) const;

    Tile& GetTile(TilePosition position);
    Tile& GetTile(int index);
    [[nodiscard]] int GetTileIndex(TilePosition position) const;

    void SetTile(TilePosition position, Tile tile);
    void RemoveTile(TilePosition position);

    [[nodiscard]] std::vector<TilePosition> GetTiles(TileType type) const;
	[[nodiscard]] std::vector<TilePosition> GetTiles(TileType type, TilePosition position, int radius) const;

	void ForEachTile(std::function<void(Tile&, TilePosition)> callback) const;

	Texture GetTexture(TilePosition position);

	static float GetMaxConstructionProgress(TileType type);
	static float GetMaxDestructionProgress(TileType type);
	static int GetMaxLogsStored(Tile tile);
	static int GetMaxRocksStored(Tile tile);

	bool CanBuild(TilePosition position, TileType type);
};