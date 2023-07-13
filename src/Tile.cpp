#include "Tile.h"

Tile::Tile(Vector2I position, int tileSize, bool isWalkable)
{
	DrawableObject::Position = position;
	DrawableObject::Size = { tileSize, tileSize };
	IsWalkable = isWalkable;
}
