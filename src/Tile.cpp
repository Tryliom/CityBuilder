#include "Tile.h"

Tile::Tile(Vector2F position, int tileSize, bool isWalkable)
{
	DrawableObject::Position = position;
	DrawableObject::Size = { static_cast<float>(tileSize), static_cast<float>(tileSize) };
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
