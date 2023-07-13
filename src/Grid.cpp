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

    for (int x = 0; x < width / tileSize; x++)
    {
        for (int y = 0; y < height / tileSize; y++)
        {
            _tiles[x + y * width] = Tile();
        }
    }
}

Texture Grid::getTexture(Tile& tile) const
{
    switch (tile.Type)
    {
        case TileType::None:
            return {};
        case TileType::Tree:
            if (tile.TreeGrowth < 15.f)
            {
                return Texture(Ressources::TreeSprout);
            }
            else if (tile.TreeGrowth < 30.f)
            {
                return Texture(Ressources::TreeMiddle);
            }
            else
            {
                return Texture(Ressources::TreeFull);
            }
        case TileType::Sawmill:
            return Texture(Buildings::Sawmill);
        case TileType::Road:
            return Texture(Road::SingleRoad); //TODO: Ici on peut faire un switch pour avoir les bonnes textures Constantin
        default:
            return {};
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
            auto background = tile.Type != TileType::None ? Texture(Land::Grass) : randomLand;

            if (tile.Type != TileType::Road)
            {
                Window::DrawObject({
                    .Position = position,
                    .Size = size,
                    .Texture = background
                });
            }

            if (tile.Type != TileType::None)
            {
                Window::DrawObject({
                    .Position = position,
                    .Size = size,
                    .Texture = getTexture(tile)
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

            if (tile.Type == TileType::Tree && tile.TreeGrowth < 30.f)
            {
                tile.TreeGrowth += smoothDeltaTime;
            }
        }
    }
}

TilePosition Grid::GetTilePosition(Vector2F position) const
{
    return TilePosition{
        (int) (position.X + _width / 2.f) / _tileSize,
        (int) (position.Y + _height / 2.f) / _tileSize
    };
}

Vector2F Grid::ToWorldPosition(TilePosition position) const
{
    return Vector2F{
        (float) position.X * _tileSize - _width / 2.f,
        (float) position.Y * _tileSize - _height / 2.f
    };
}

Tile& Grid::GetTile(TilePosition position)
{
    return _tiles[position.X + position.Y * _width];
}

Tile& Grid::GetTile(int index)
{
    return _tiles[index];
}

int Grid::GetTileIndex(TilePosition position) const
{
    return position.X + position.Y * _width;
}

void Grid::SetTile(TilePosition position, Tile tile)
{
    _tiles[position.X + position.Y * _width] = tile;
}

void Grid::RemoveTile(TilePosition position)
{
    _tiles[position.X + position.Y * _width] = Tile();
}

std::vector<TilePosition> Grid::GetTiles(TileType type) const
{
    std::vector<TilePosition> tiles;

    for (int x = 0; x < _width / _tileSize; x++)
    {
        for (int y = 0; y < _height / _tileSize; y++)
        {
            Tile& tile = _tiles[x + y * _width];

            if (tile.Type == type)
            {
                tiles.push_back(TilePosition{ x, y });
            }
        }
    }

    return tiles;
}
