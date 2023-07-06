#pragma once

#include "Maths.h"
#include "Texture.h"
#include "Color.h"

struct DrawableObject
{
    Vector2F Position = {0, 0};
    Vector2F Pivot = {0, 0};
    Vector2F Scale = {1, 1};
    Vector2F Size = {0, 0};
    float Rotation = 0;
    Color Color{ 1.f, 1.f, 1.f, 1.f };

    bool UseTexture = false;
    TextureName TextureName = TextureName::Center;
};