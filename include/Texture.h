#pragma once

enum class TileMap
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
    explicit Texture(Road road) : TileMapIndex(TileMap::Road), TileIndex((int) road) {}
    explicit Texture(Buildings buildings) : TileMapIndex(TileMap::Buildings), TileIndex((int) buildings) {}
    explicit Texture(Ressources ressources) : TileMapIndex(TileMap::Ressources), TileIndex((int) ressources) {}
    explicit Texture(Characters characters) : TileMapIndex(TileMap::Characters), TileIndex((int) characters) {}
    explicit Texture(Icons icons) : TileMapIndex(TileMap::Icons), TileIndex((int) icons) {}

    TileMap TileMapIndex = TileMap::None;
    int TileIndex = -1;
};