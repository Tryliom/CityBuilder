#include "Grid.h"
#include "Window.h"
#include "Random.h"
#include "Input.h"
#include "Timer.h"

#include <iostream>

Grid::Grid(int width, int height, int tileSize)
{
    _width = width;
    _height = height;
    _tileSize = tileSize;

	_tiles = (Tile*) malloc(sizeof(Tile) * width * height);

    for (int x = 0; x < _width; x++)
    {
        for (int y = 0; y < _height; y++)
        {
            Tile& tile = _tiles[x + y * _width];

            tile.Texture = Texture();
        }
    }
}

void Grid::Draw()
{
    Random::SetSeed(42);
    Random::UseSeed();

    auto mousePosition = Input::GetMousePosition();

    std::cout << "Screen " << mousePosition.X << " " << mousePosition.Y << std::endl;

    Vector2F worldMousePosition = Window::ScreenToWorld(mousePosition);

    std::cout << "World  " << worldMousePosition.X << " " << worldMousePosition.Y << std::endl;

    for (int x = 0; x < _width / _tileSize; x++)
    {
        for (int y = 0; y < _height / _tileSize; y++)
        {
            Tile& tile = _tiles[x + y * _width];
            auto position = Vector2F{ x, y } * _tileSize - Vector2F{ _width, _height} / 2.f;
            auto size = Vector2F{ (float) _tileSize, (float) _tileSize};
            auto randomLand = Texture((Land) Random::Range(0, (int) Land::Count - 1));
            Texture background = tile.Texture.TileSheetIndex == TileSheet::None ? randomLand : Texture(Land::Grass);

            if (tile.Texture.TileSheetIndex != TileSheet::Road)
            {
                Window::DrawObject({
                    .Position = position,
                    .Size = size,
                    .Texture = background
                });
            }

            if (tile.Texture.TileSheetIndex != TileSheet::None)
            {
                Window::DrawObject({
                    .Position = position,
                    .Size = size,
                    .Texture = tile.Texture
                });
            }

            

            if (position.X < worldMousePosition.X && position.X + _tileSize > worldMousePosition.X &&
                position.Y < worldMousePosition.Y && position.Y + _tileSize > worldMousePosition.Y)
            {
                Window::DrawRect(
                    position,
                    Vector2F{ (float) _tileSize, (float) _tileSize},
                    Color(1, 1, 1, 0.2f)
                );
            }
        }
    }

    Random::StopUseSeed();
}

void Grid::Update()
{
    float smoothDeltaTime = Timer::SmoothDeltaTime;

    // Check tree
    for (int x = 0; x < _width / _tileSize; x++)
    {
        for (int y = 0; y < _height / _tileSize; y++)
        {
            Tile& tile = _tiles[x + y * _width];

            if (tile.Texture == Ressources::TreeSprout)
            {
                tile.TreeGrowth += smoothDeltaTime;

                if (tile.TreeGrowth >= 15.f)
                {
                    tile.Texture = Texture(Ressources::TreeMiddle);
                }
            }
            else if (tile.Texture == Ressources::TreeMiddle)
            {
                tile.TreeGrowth += smoothDeltaTime;

                if (tile.TreeGrowth >= 30.f)
                {
                    tile.Texture = Texture(Ressources::TreeFull);
                }
            }
        }
    }
}

Vector2I Grid::GetTilePosition(Vector2F position) const
{
    return Vector2I{
        (int) (position.X + _width / 2.f) / _tileSize,
        (int) (position.Y + _height / 2.f) / _tileSize
    };
}

void Grid::SetTile(Vector2I position, Tile tile)
{
    _tiles[position.X + position.Y * _width] = tile;
}

void Grid::RemoveTile(Vector2I position)
{
    _tiles[position.X + position.Y * _width] = Tile();
}