#include "Tile.h"

Tile::Tile(Vector2I position, int tileSize, bool isWalkable)
{
	DrawableObject::Position = position;
	DrawableObject::Size = { tileSize, tileSize };
	IsWalkable = isWalkable;
}

void Tile::SetSelected(bool selected)
{
	if (selected)
	{
		DrawableObject::Color = Color::Red;
	}
	else
	{
		DrawableObject::Color = Color::White;
	}
}
