#pragma once

#include <map>

#include "Texture.h"
#include "Maths.h"

struct TilePosition;

enum class UnitBehavior
{
    Idle,
    Moving,
    Working, // Example: if it's a lumberjack, it's cutting a tree
    Count
};

struct Unit
{
	explicit Unit(Vector2F position)
	{
		Position = position;
	}

    UnitBehavior CurrentBehavior = UnitBehavior::Idle;

    int JobTileIndex = -1;
    Vector2F Position {};
    TilePosition TargetTile {};

    float TimeSinceLastAction = 0.f;

	// Inventory
	std::map<Items, int>* Inventory = new std::map<Items, int>
	{
		{ Items::Wood, 0 },
		{ Items::Stone, 0 }
	};

    void SetBehavior(UnitBehavior behavior)
    {
        CurrentBehavior = behavior;
        TimeSinceLastAction = 0.f;
    }
};