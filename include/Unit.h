#pragma once

#include <map>
#include <vector>

#include "Texture.h"
#include "Maths.h"
#include "Grid.h"

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
	std::vector<TilePosition> PathToTargetTile {};

    float TimeSinceLastAction = 0.f;

	// Inventory
	std::map<Items, int>* Inventory = new std::map<Items, int>
	{
		{ Items::Wood, 0 },
		{ Items::Stone, 0 },
		{ Items::Coal, 0 },
		{ Items::IronOre, 0 },
		{ Items::IronIngot, 0 }
	};

    void SetBehavior(UnitBehavior behavior)
    {
        CurrentBehavior = behavior;
        TimeSinceLastAction = 0.f;
		PathToTargetTile.clear();
    }
};