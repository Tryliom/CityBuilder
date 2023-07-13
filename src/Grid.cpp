#include "Grid.h"
#include "Window.h"

Grid::Grid(int width, int height, int tileSize)
{
    _width = width;
    _height = height;
    _tileSize = tileSize;

	for (int x = 0; x < width / tileSize; x++)
	{
		for (int y = 0; y < height / tileSize; y++)
		{
			Tiles.emplace_back(
                    Vector2F { x * tileSize, y * tileSize} - Vector2F{ _width, _height} / 2.f,
                    tileSize
            );
		}
	}
}

void Grid::Draw()
{
    for (Tile tile : Tiles)
    {
        Window::DrawObject({
            .Position = tile.Position,
            .Size = Vector2F{ (float) _tileSize, (float) _tileSize},
            .Texture = tile.Texture
        });

        if (tile.IsSelected)
        {
            Window::DrawRect(
                tile.Position,
                Vector2F{ (float) _tileSize, (float) _tileSize},
                Color(1, 1, 1, 0.2f)
            );
        }
    }
}