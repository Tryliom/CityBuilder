#include "Grid.h"
#include "Window.h"
#include "Random.h"
#include "Input.h"

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

            auto mousePosition = Input::GetMousePosition();

            if (position.X < mousePosition.X && position.X + _tileSize > mousePosition.X &&
                position.Y < mousePosition.Y && position.Y + _tileSize > mousePosition.Y)
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

void Grid::SetTile(int x, int y, Tile tile)
{
    _tiles[x + y * _width] = tile;
}

void Grid::RemoveTile(int x, int y)
{
    _tiles[x + y * _width] = Tile();
}