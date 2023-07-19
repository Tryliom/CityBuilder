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
	// When it met 30.f, it has a chance to spawn a new tree around it
	float TreeSpawnTimer = 0.f;

    // Storage
    std::map<Items, int>* Inventory = new std::map<Items, int>
	{
		{ Items::Wood, 0 },
		{ Items::Stone, 0 },
		{ Items::Coal, 0 },
		{ Items::IronOre, 0 },
		{ Items::IronIngot, 0 }
	};

	void Reset()
	{
		Type = TileType::None;
		Progress = 0.f;
		IsBuilt = false;
		NeedToBeDestroyed = false;
		TreeGrowth = 0.f;
		TreeSpawnTimer = 0.f;

		for (auto& item : *Inventory)
		{
			item.second = 0;
		}
	}

    [[nodiscard]] int GetInventorySize() const
    {
        int size = 0;

        for (auto& item : *Inventory)
        {
            size += item.second;
        }

        return size;
    }
};