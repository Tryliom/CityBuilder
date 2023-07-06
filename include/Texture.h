#pragma once

struct Texture
{
    int X = 0;
    int Y = 0;
    int Width = 0;
    int Height = 0;
};

enum class TextureName
{
    BottomLeft, Bottom, BottomRight,
    CenterLeft, Center, CenterRight,
    TopLeft, Top, TopRight
};