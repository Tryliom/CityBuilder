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

	[[nodiscard]] int GetTileSize() const { return _tileSize; }

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
	[[nodiscard]] std::vector<TilePosition> GetTiles(TilePosition position, int radius) const;
	[[nodiscard]] std::vector<TilePosition> GetTilesWithItems(TileType type, Items item) const;

	void ForEachTile(const std::function<void(Tile&, TilePosition)>& callback) const;

	Texture GetTexture(TilePosition position);

	static float GetMaxConstructionProgress(TileType type);
	static float GetMaxDestructionProgress(TileType type);

	static int GetMaxItemsStored(const Tile& tile, Items item);
	static int GetLeftSpaceForItems(Tile tile, Items item);
	static int GetNeededItemsToBuild(TileType type, Items item);
	static bool IsTileReadyToBuild(Tile& tile);
	static bool IsAStorage(TileType type);

	// Stats for units
	static float GetSpeedFactor(TileType type);

	bool CanBuild(TilePosition position, TileType type);

	// Pathfinding
	std::vector<TilePosition> GetPath(TilePosition start, TilePosition end);
	[[nodiscard]] std::vector<TilePosition> GetNeighbors(TilePosition position) const;
};