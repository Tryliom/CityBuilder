#pragma once

#include "Texture.h"
#include "Tile.h"
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
    UnitBehavior CurrentBehavior = UnitBehavior::Idle;

    int JobTileIndex = -1;
    Vector2F Position {};
    TilePosition TargetTile {};

    float TimeSinceLastAction = 0.f;

    void SetBehavior(UnitBehavior behavior)
    {
        CurrentBehavior = behavior;
        TimeSinceLastAction = 0.f;
    }
};