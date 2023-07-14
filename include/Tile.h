#pragma once

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
	// Resources
    Tree,
    Stone,
    Road,
    Count
};

struct Tile
{
    TileType Type = TileType::None;

    // TODO: Add more properties like harvestable, etc.

	// Constructor
	float Progress = 0.f;
	bool IsBuilt = false;
	bool NeedToBeDestroyed = false;

    // Tree
    float TreeGrowth = 0.f;

    // Storage
    int Logs = 0;
	int Rocks = 0;
};