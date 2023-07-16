#pragma once

#include <map>
#include "Texture.h"

enum class TileType
{
    None,
	// Buildings
	MayorHouse,
	House,
	BuilderHut,
	Storage,
	Sawmill,
	Quarry,
	LogisticsCenter,
	// Resources
    Tree,
    Stone,
    Road,
    Count
};

struct Tile
{
    TileType Type = TileType::None;

	// Constructor
	float Progress = 0.f;
	bool IsBuilt = false;
	bool NeedToBeDestroyed = false;

    // Tree
    float TreeGrowth = 0.f;

    // Storage
    std::map<Items, int> Inventory =
	{
		{ Items::Wood, 0 },
		{ Items::Stone, 0 }
	};

	void Reset()
	{
		Type = TileType::None;
		Progress = 0.f;
		IsBuilt = false;
		NeedToBeDestroyed = false;
		Inventory[Items::Wood] = 0;
		Inventory[Items::Stone] = 0;
		TreeGrowth = 0.f;
	}
};