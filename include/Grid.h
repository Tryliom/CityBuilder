#pragma once

#include <utility>
#include <vector>
#include <functional>

#include "Tile.h"
#include "Maths.h"
#include "Serialization.h"

struct TilePosition
{
    int X = -1;
    int Y = -1;

    bool operator==(const TilePosition& other) const
    {
        return X == other.X && Y == other.Y;
    }

	TilePosition operator+(const TilePosition& other) const
	{
		return TilePosition{ X + other.X, Y + other.Y };
	}

    [[nodiscard]] float GetDistance(TilePosition other) const
    {
        return Vector2F{ X, Y }.GetDistance(Vector2F{ other.X, other.Y });
    }
};

class Grid
{
public:
	Grid(int width, int height, int tileSize);

	int _width;
	int _height;
	int _tileSize;

	Tile* _tiles;

private:

	// Texture
	static Texture getTreeTexture(Tile& tile);
	Texture getRoadTexture(TilePosition position);

public:
    void Draw(bool drawLandAndRoads, bool isMouseOnAWindow);
    void Update();

	[[nodiscard]] int GetTileSize() const { return _tileSize; }

	[[nodiscard]] TilePosition GetTilePosition(Vector2F position) const;
	[[nodiscard]] TilePosition GetTilePosition(int tileIndex) const;
    [[nodiscard]] Vector2F ToWorldPosition(TilePosition position) const;

    Tile& GetTile(TilePosition position);
    Tile& GetTile(int index);
    [[nodiscard]] int GetTileIndex(TilePosition position) const;
    [[nodiscard]] bool IsTileValid(TilePosition position) const;
	bool IsRoad(TilePosition tp);

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
	bool CanBeDestroyed(TilePosition position);

	// Pathfinding
	std::vector<TilePosition> GetPath(TilePosition start, TilePosition end);
	[[nodiscard]] std::vector<TilePosition> GetNeighbours(TilePosition position) const;
};


struct Serializer;
void Serialize(Serializer* ser, Grid* grid);