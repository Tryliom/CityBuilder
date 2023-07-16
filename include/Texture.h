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
    Single, Vertical, Horizontal,
	TopRightCorner, BottomRightCorner, TopLeftCorner, BottomLeftCorner,
	Cross,
	TopRightCornerEmpty, BottomRightCornerEmpty, TopLeftCornerEmpty, BottomLeftCornerEmpty,
	Empty, BottomEnd, LeftEnd, TopEnd, RightEnd,
	TopEmpty, TopT, TopTLeftEmpty, TopTRightEmpty,
	RightEmpty, RightT, RightTTopEmpty, RightTBottomEmpty,
	BottomEmpty, BottomT, BottomTLeftEmpty, BottomTRightEmpty,
	LeftEmpty, LeftT, LeftTTopEmpty, LeftTBottomEmpty,
	CrossTopLeftEmpty, CrossTopRightEmpty, CrossBottomRightEmpty, CrossBottomLeftEmpty,
	CrossTopEmpty, CrossBottomEmpty, CrossRightEmpty, CrossLeftEmpty,
	CrossBottomLeftToTopRightEmpty, CrossBottomRightToTopLeftEmpty,
	CrossBottomLeft, CrossBottomRight, CrossTopRight, CrossTopLeft,
	Count
};

enum class Buildings
{
    MayorHouse, House, Sawmill, Quarry, Storage, BuilderHut, LogisticsCenter, Count
};

enum class Ressources
{
    TreeSprout, TreeMiddle, TreeFull, Stone, Count
};

enum class Characters
{
    Unemployed, Lumberjack, Digger, Builder, Logistician, Count
};

enum class Icons
{
    Wood, Stone, People, Count // People need to be last
};

enum class Items
{
	Wood, Stone, Count
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
	explicit Texture(Items items) : TileSheetIndex(TileSheet::Icons), TileIndex((int) items) {}

    TileSheet TileSheetIndex = TileSheet::None;
    int TileIndex = -1;

    bool operator==(Land land) const { return TileSheetIndex == TileSheet::Land && TileIndex == (int) land; }
    bool operator==(Road road) const { return TileSheetIndex == TileSheet::Road && TileIndex == (int) road; }
    bool operator==(Buildings buildings) const { return TileSheetIndex == TileSheet::Buildings && TileIndex == (int) buildings; }
    bool operator==(Ressources ressources) const { return TileSheetIndex == TileSheet::Ressources && TileIndex == (int) ressources; }
    bool operator==(Characters characters) const { return TileSheetIndex == TileSheet::Characters && TileIndex == (int) characters; }
    bool operator==(Icons icons) const { return TileSheetIndex == TileSheet::Icons && TileIndex == (int) icons; }
};