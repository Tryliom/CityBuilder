
struct Tile;

template <class T>
struct Vector2;
using Vector2F = Vector2<float>;

typedef void* ImTextureID; 

namespace GUI
{
    void DrawStartMenu(bool* gameStarted);

    void DrawTileInventory(Tile& tile);

    void DrawConstructionMenu(int* buildingSelected, Vector2F* screenSize, ImTextureID* imTilemapTextureID);
};