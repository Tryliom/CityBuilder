#pragma once

enum class TileSheet
{
    None = -1, Land, Road, Buildings, Ressources, Characters, Icons, Count
};

enum class Land
{
    Grass, Flower1, Flower2, Flower3, Count
};

enum class Road
{
    SingleRoad, VerticalRoad, HorizontalRoad, TopRightCorner, BottomRightCorner, TopLeftCorner,
    BottomLeftCorner, CrossRoad, Count
};

enum class Buildings
{
    MainHouse, CommonHouse, Sawmill, Quarry, Storage, BuilderHut, Count
};

enum class Ressources
{
    TreeSprout, TreeMiddle, TreeFull, Stone, Count
};

enum class Characters
{
    Unemployed, Lumberjack, Digger, Builder, Count
};

enum class Icons
{
    Wood, Stone, People, Count
};

struct Texture
{
    Texture() = default;
    explicit Texture(Land land) : TileSheetIndex(TileSheet::Land), TileIndex((int) land) {}
    explicit Texture(Road road) : TileSheetIndex(TileSheet::Road), TileIndex((int) road) {}
    explicit Texture(Buildings buildings) : TileSheetIndex(TileSheet::Buildings), TileIndex((int) buildings) {}
    explicit Texture(Ressources ressources) : TileSheetIndex(TileSheet::Ressources), TileIndex((int) ressources) {}
    explicit Texture(Characters characters) : TileSheetIndex(TileSheet::Characters), TileIndex((int) characters) {}
    explicit Texture(Icons icons) : TileSheetIndex(TileSheet::Icons), TileIndex((int) icons) {}

    TileSheet TileSheetIndex = TileSheet::None;
    int TileIndex = -1;
};