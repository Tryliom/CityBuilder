#pragma once

#include "Texture.h"

enum class TileType
{
    None,
    Tree,
    Sawmill,
    Road,
    Count
};

struct Tile
{
    TileType Type = TileType::None;

    // TODO: Add more properties like harvestable, etc.

    // Tree
    float TreeGrowth = 0.f;

    // Sawmill
    int Logs = 0;
};