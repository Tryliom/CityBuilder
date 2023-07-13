#pragma once

#include "Texture.h"
#include "Maths.h"
#include "DrawableObject.h"

class Tile : public DrawableObject
{
public:
	explicit Tile(Vector2I position, int tileSize, bool isWalkable = true);

	bool IsWalkable = true;
    bool IsSelected = false;
};