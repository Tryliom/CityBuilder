#pragma once

enum class TileSheet
{
    None = -1, Road, Buildings, Ressources, Characters, Icons
};

enum class Road
{
    Grass, Flower1, Flower2, Flower3, SingleRoad
};

enum class Buildings
{
    MainHouse, CommonHouse,
};

enum class Ressources
{
    TreeSprout, TreeMiddle, TreeFull, Stone
};

enum class Characters
{
    Unemployed, Lumberjack, Digger, Builder
};

enum class Icons
{
    Wood, Stone, People
};

struct Texture
{
    Texture() = default;
    explicit Texture(Road road) : TileSheetIndex(TileSheet::Road), TileIndex((int) road) {}
    explicit Texture(Buildings buildings) : TileSheetIndex(TileSheet::Buildings), TileIndex((int) buildings) {}
    explicit Texture(Ressources ressources) : TileSheetIndex(TileSheet::Ressources), TileIndex((int) ressources) {}
    explicit Texture(Characters characters) : TileSheetIndex(TileSheet::Characters), TileIndex((int) characters) {}
    explicit Texture(Icons icons) : TileSheetIndex(TileSheet::Icons), TileIndex((int) icons) {}

    TileSheet TileSheetIndex = TileSheet::None;
    int TileIndex = -1;
};