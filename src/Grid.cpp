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

Texture Grid::GetTexture(Tile& tile) const
{
    switch (tile.Type)
    {
        case TileType::Stone: return Texture(Ressources::Stone);
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
        case TileType::Sawmill: return Texture(Buildings::Sawmill);
        case TileType::Road: return Texture(Road::SingleRoad); //TODO: Ici on peut faire un switch pour avoir les bonnes textures Constantin
	    case TileType::MayorHouse: return Texture(Buildings::MayorHouse);
		case TileType::House: return Texture(Buildings::House);
		case TileType::BuilderHut: return Texture(Buildings::BuilderHut);
		case TileType::Storage: return Texture(Buildings::Storage);
		case TileType::Quarry: return Texture(Buildings::Quarry);
        default: return {};
    }
}

void Grid::Draw()
{
    Random::UseSeed();

    auto mousePosition = Input::GetMousePosition();
    Vector2F worldMousePosition = Window::ScreenToWorld(mousePosition);

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
				if (!tile.IsBuilt)
				{
					Window::DrawRect(position, size, Color(1, 1, 0, 1.f - tile.Progress / GetMaxConstructionProgress(tile.Type)));
				}
				else if (tile.NeedToBeDestroyed)
				{
					Window::DrawRect(position, size, Color(1, 0, 0, 1.f - tile.Progress / GetMaxDestructionProgress(tile.Type)));
				}

                Window::DrawObject({
                    .Position = position,
                    .Size = size,
                    .Texture = GetTexture(tile)
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

    for (int x = 0; x < _width / _tileSize; x++)
    {
        for (int y = 0; y < _height / _tileSize; y++)
        {
            Tile& tile = _tiles[x + y * _width];

	        // Check tree
            if (tile.Type == TileType::Tree && tile.TreeGrowth < 30.f)
            {
                tile.TreeGrowth += smoothDeltaTime;
            }

			// Check construction
			if (!tile.IsBuilt && tile.Type != TileType::None)
			{
				tile.IsBuilt = GetMaxConstructionProgress(tile.Type) <= tile.Progress;
			}

			// Check destruction
			if (tile.IsBuilt && tile.Type != TileType::None && tile.NeedToBeDestroyed)
			{
				if (GetMaxDestructionProgress(tile.Type) <= tile.Progress)
				{
					tile.Type = TileType::None;
					tile.IsBuilt = false;
					tile.NeedToBeDestroyed = false;
					tile.Progress = 0.f;
				}
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

[[nodiscard]] TilePosition Grid::GetTilePosition(int tileIndex) const
{
	return TilePosition{
		tileIndex % _width,
		tileIndex / _width
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
	// Set the tile as build if it's not a building
	if (tile.Type != TileType::Storage && tile.Type != TileType::BuilderHut && tile.Type != TileType::Quarry && tile.Type != TileType::Sawmill && tile.Type != TileType::House && tile.Type != TileType::MayorHouse)
	{
		tile.IsBuilt = true;
	}

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

            if (tile.Type == type && tile.IsBuilt)
            {
                tiles.push_back(TilePosition{ x, y });
            }
        }
    }

    return tiles;
}

std::vector<TilePosition> Grid::GetTiles(TileType type, TilePosition position, int radius) const
{
	std::vector<TilePosition> tiles;

	for (int x = position.X - radius; x < position.X + radius; x++)
	{
		for (int y = position.Y - radius; y < position.Y + radius; y++)
		{
			Tile& tile = _tiles[x + y * _width];

			if (tile.Type == type && tile.IsBuilt)
			{
				tiles.push_back(TilePosition{ x, y });
			}
		}
	}

	return tiles;
}

void Grid::ForEachTile(std::function<void(Tile&, TilePosition)> callback) const
{
	for (int x = 0; x < _width / _tileSize; x++)
	{
		for (int y = 0; y < _height / _tileSize; y++)
		{
			Tile& tile = _tiles[x + y * _width];
			callback(tile, TilePosition{ x, y });
		}
	}
}

float Grid::GetMaxConstructionProgress(TileType type)
{
	switch (type)
	{
		case TileType::Sawmill: return 10.f;
		case TileType::Quarry: return 15.f;
		case TileType::BuilderHut: return 10.f;
		case TileType::Storage: return 10.f;
		case TileType::House: return 30.f;
	}
}

float Grid::GetMaxDestructionProgress(TileType type)
{
	switch (type)
	{
		case TileType::Sawmill: return 5.f;
		case TileType::Quarry: return 7.5f;
		case TileType::BuilderHut: return 5.f;
		case TileType::Storage: return 5.f;
		case TileType::House: return 15.f;
		case TileType::Tree: return 5.f;
		case TileType::Stone: return 10.f;
	}
}


