#include "map"

#include "Grid.h"
#include "Graphics.h"
#include "Random.h"
#include "Input.h"
#include "Timer.h"
#include "Logger.h"

std::map<TileType, std::map<Items, int>> *tileMaxInventory = new std::map<TileType, std::map<Items, int>>{
    {TileType::Sawmill, {{Items::Wood, 50}}},
    {TileType::Storage, {{Items::Wood, 100}, {Items::Stone, 100}, {Items::Coal, 50}, {Items::IronOre, 100}, {Items::IronIngot, 20}}},
    {TileType::LogisticsCenter, {{Items::Wood, 50}, {Items::Stone, 50}, {Items::Coal, 25}, {Items::IronOre, 50}, {Items::IronIngot, 10}}},
    {TileType::Quarry, {{Items::Stone, 50}, {Items::Coal, 25}, {Items::IronOre, 50}}},
    {TileType::MayorHouse, {{Items::Wood, 15}, {Items::Stone, 15}}},
    {TileType::Furnace, {{Items::Coal, 10}, {Items::IronOre, 60}, {Items::IronIngot, 20}}},
};

// Items needed to build a tile
std::map<TileType, std::map<Items, int>> tileNeededItems = std::map<TileType, std::map<Items, int>>{
    {TileType::Sawmill, {{Items::Wood, 10}}},
    {TileType::Storage, {{Items::Wood, 20}}},
    {TileType::LogisticsCenter, {{Items::Wood, 20}, {Items::Stone, 10}}},
    {TileType::Quarry, {{Items::Wood, 20}}},
    {TileType::House, {{Items::Wood, 30}, {Items::Stone, 15}}},
    {TileType::BuilderHut, {{Items::Wood, 20}, {Items::Stone, 10}}},
    {TileType::Furnace, {{Items::Stone, 35}}},
};

Grid::Grid(int width, int height, int tileSize)
{
    _width = width;
    _height = height;
    _tileSize = tileSize;

    _tiles = (Tile *)malloc(sizeof(Tile) * width * height);

    for (int x = 0; x < width / tileSize; x++)
    {
        for (int y = 0; y < height / tileSize; y++)
        {
            _tiles[x + y * width] = Tile(TileType::None);
        }
    }
}

Texture Grid::GetTexture(TilePosition position)
{
    Tile &tile = GetTile(position);

    switch (tile.Type)
    {
        case TileType::Stone: return Texture(Resources::Stone);
        case TileType::Tree: return getTreeTexture(tile);
        case TileType::Sawmill: return Texture(Buildings::Sawmill);
        case TileType::Road: return getRoadTexture(position);
        case TileType::MayorHouse: return Texture(Buildings::MayorHouse);
        case TileType::House: return Texture(Buildings::House);
        case TileType::BuilderHut: return Texture(Buildings::BuilderHut);
        case TileType::Storage: return Texture(Buildings::Storage);
        case TileType::Quarry: return Texture(Buildings::Quarry);
        case TileType::LogisticsCenter: return Texture(Buildings::LogisticsCenter);
        case TileType::Furnace: return tile.BurnTimer > 0.f ? Texture(Buildings::ActiveFurnace) : Texture(Buildings::InactiveFurnace);
        default: return {};
    }
}

Texture Grid::getTreeTexture(Tile &tile)
{
    if (tile.TreeGrowth < 15.f)
    {
        return Texture(Resources::TreeSprout);
    }
    else if (tile.TreeGrowth < 30.f)
    {
        return Texture(Resources::TreeMiddle);
    }
    else
    {
        return Texture(Resources::TreeFull);
    }
}
bool Grid::IsRoad(TilePosition tp)
{
    return IsTileValid(tp) && GetTile(tp).Type == TileType::Road;
}

Texture Grid::getRoadTexture(TilePosition position)
{
    bool up = IsRoad(position + TilePosition{0, -1});
    bool down = IsRoad(position + TilePosition{0, 1});
    bool left = IsRoad(position + TilePosition{-1, 0});
    bool right = IsRoad(position + TilePosition{1, 0});
    bool upLeft = IsRoad(position + TilePosition{-1, -1});
    bool upRight = IsRoad(position + TilePosition{1, -1});
    bool downLeft = IsRoad(position + TilePosition{-1, 1});
    bool downRight = IsRoad(position + TilePosition{1, 1});

    if (up && down && left && right && !upLeft && !upRight && !downLeft && !downRight) // cross
        return Texture(Road::Cross);
    else if (up && down && left && right && upLeft && upRight && downLeft && downRight) // full
        return Texture(Road::Empty);
    else if (up && down && left && right && !upLeft && upRight && downLeft && !downRight) // diagonal corners empty
        return Texture(Road::CrossBottomLeftToTopRightEmpty);
    else if (up && down && left && right && upLeft && !upRight && !downLeft && downRight)
        return Texture(Road::CrossBottomRightToTopLeftEmpty);
    else if (up && down && left && right && upLeft && upRight && !downLeft && downRight) // 1 corner empty
        return Texture(Road::CrossBottomLeft);
    else if (up && down && left && right && upLeft && !upRight && downLeft && downRight)
        return Texture(Road::CrossTopRight);
    else if (up && down && left && right && !upLeft && upRight && downLeft && downRight)
        return Texture(Road::CrossTopLeft);
    else if (up && down && left && right && upLeft && upRight && downLeft && !downRight)
        return Texture(Road::CrossBottomRight);
    else if (up && down && left && right && upLeft && upRight && !downLeft && !downRight) // 2 corners empty
        return Texture(Road::CrossTopEmpty);
    else if (up && down && left && right && !upLeft && !upRight && downLeft && downRight)
        return Texture(Road::CrossBottomEmpty);
    else if (up && down && left && right && upLeft && !upRight && downLeft && !downRight)
        return Texture(Road::CrossLeftEmpty);
    else if (up && down && left && right && !upLeft && upRight && !downLeft && downRight)
        return Texture(Road::CrossRightEmpty);
    else if (up && down && left && right && upLeft && !upRight && !downLeft && !downRight) // 3 corners empty
        return Texture(Road::CrossTopLeftEmpty);
    else if (up && down && left && right && !upLeft && upRight && !downLeft && !downRight)
        return Texture(Road::CrossTopRightEmpty);
    else if (up && down && left && right && !upLeft && !upRight && !downLeft && downRight)
        return Texture(Road::CrossBottomRightEmpty);
    else if (up && down && left && right && !upLeft && !upRight && downLeft && !downRight)
        return Texture(Road::CrossBottomLeftEmpty);
    else if (up && down && !left && right && upRight && downRight) // 1 side empty
        return Texture(Road::LeftEmpty);
    else if (up && down && left && !right && upLeft && downLeft)
        return Texture(Road::RightEmpty);
    else if (!up && down && left && right && downLeft && downRight)
        return Texture(Road::TopEmpty);
    else if (up && !down && left && right && upLeft && upRight)
        return Texture(Road::BottomEmpty);
    else if (up && down && !left && right && !upRight && !downRight) // 1 side empty T
        return Texture(Road::LeftT);
    else if (up && down && left && !right && !upLeft && !downLeft)
        return Texture(Road::RightT);
    else if (!up && down && left && right && !downLeft && !downRight)
        return Texture(Road::TopT);
    else if (up && !down && left && right && !upLeft && !upRight)
        return Texture(Road::BottomT);
    else if (up && down && !left && right && upRight && !downRight) // 1 side empty top corner empty
        return Texture(Road::LeftTTopEmpty);
    else if (up && down && left && !right && upLeft && !downLeft)
        return Texture(Road::RightTTopEmpty);
    else if (up && down && !left && right && !upRight && downRight) // 1 side empty bottom corner empty
        return Texture(Road::LeftTBottomEmpty);
    else if (up && down && left && !right && !upLeft && downLeft)
        return Texture(Road::RightTBottomEmpty);
    else if (!up && down && left && right && downLeft && !downRight) // 1 side empty right corner empty
        return Texture(Road::TopTLeftEmpty);
    else if (!up && down && left && right && !downLeft && downRight)
        return Texture(Road::TopTRightEmpty);
    else if (up && !down && left && right && upLeft && !upRight) // 1 side empty left corner empty
        return Texture(Road::BottomTLeftEmpty);
    else if (up && !down && left && right && !upLeft && upRight)
        return Texture(Road::BottomTRightEmpty);
    else if (!up && down && left && !right && downLeft) // corners full
        return Texture(Road::TopRightCornerEmpty);
    else if (!up && down && !left && right && downRight)
        return Texture(Road::TopLeftCornerEmpty);
    else if (up && !down && left && !right && upLeft)
        return Texture(Road::BottomRightCornerEmpty);
    else if (up && !down && !left && right && upRight)
        return Texture(Road::BottomLeftCornerEmpty);
    else if (!up && down && left && !right && !downLeft) // corners full opposite corner empty
        return Texture(Road::TopRightCorner);
    else if (!up && down && !left && right && !downRight)
        return Texture(Road::TopLeftCorner);
    else if (up && !down && left && !right && !upLeft)
        return Texture(Road::BottomRightCorner);
    else if (up && !down && !left && right && !upRight)
        return Texture(Road::BottomLeftCorner);
    else if (!up && down && !left && !right) // ends
        return Texture(Road::TopEnd);
    else if (up && !down && !left && !right)
        return Texture(Road::BottomEnd);
    else if (!up && !down && left && !right)
        return Texture(Road::RightEnd);
    else if (!up && !down && !left && right)
        return Texture(Road::LeftEnd);
    else if (up && down && !left && !right) // vertical
        return Texture(Road::Vertical);
    else if (!up && !down && left && right) // horizontal
        return Texture(Road::Horizontal);

    return Texture(Road::Single);
}

void Grid::Draw(bool drawLandAndRoads, bool isMouseOnAWindow)
{
    Random::UseSeed();

    auto mousePosition = Input::GetMousePosition();
    Vector2F worldMousePosition = Graphics::ScreenToWorld(mousePosition);
	TilePosition mouse = GetTilePosition(worldMousePosition);

    for (int x = 0; x < _width / _tileSize; x++)
    {
        for (int y = 0; y < _height / _tileSize; y++)
        {
            Tile &tile = _tiles[x + y * _width];
            auto position = Vector2F{x, y} * _tileSize - Vector2F{_width, _height} / 2.f;
            auto size = Vector2F{(float)_tileSize, (float)_tileSize};
            auto randomLand = Texture((Land)Random::Range(1, (int)Land::Count - 1));
            auto background = tile.Type != TileType::None ? Texture(Land::Grass) : randomLand;

            if (tile.Type != TileType::Road && drawLandAndRoads)
            {
                Graphics::DrawObject({
                    .Position = position,
                    .Size = size,
                    .Texture = background
                });
            }

            if (tile.Type != TileType::None)
            {
				if (drawLandAndRoads)
				{
					if (!tile.IsBuilt)
					{
						Graphics::DrawRect(position, size, Color(1, 1, 0, 1.f - tile.Progress / GetMaxConstructionProgress(tile.Type)));
					}
					else if (tile.NeedToBeDestroyed)
					{
						Graphics::DrawRect(position, size, Color(1, 0, 0, 1.f - tile.Progress / GetMaxDestructionProgress(tile.Type)));
					}
				}

				if (tile.Type == TileType::Road && drawLandAndRoads || !drawLandAndRoads && tile.Type != TileType::Road)
				{
					Graphics::DrawObject({
						.Position = position,
						.Size = size,
						.Texture = GetTexture({x, y})
					});
				}
            }

            if (!drawLandAndRoads && mouse == TilePosition{x, y} && !isMouseOnAWindow)
            {
                Graphics::DrawRect(
                    position,
                    Vector2F{(float)_tileSize, (float)_tileSize},
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
            Tile &tile = _tiles[x + y * _width];

            // Check tree
            if (tile.Type == TileType::Tree && tile.TreeGrowth < 30.f)
            {
                tile.TreeGrowth += smoothDeltaTime;
            }

            // Check tree spawn, have a 3% chance to spawn a tree on a neighbour tile every 30sec
            if (tile.Type == TileType::Tree)
            {
                tile.TreeSpawnTimer += smoothDeltaTime;

                if (tile.TreeSpawnTimer >= 30.f)
                {
                    tile.TreeSpawnTimer = 0.f;

                    auto neighbours = GetNeighbours({x, y});

                    for (auto neighbour : neighbours)
                    {
                        Tile &tileNeighbour = GetTile(neighbour);

                        if (tileNeighbour.Type == TileType::None && Random::Range(0, 100) < 1)
                        {
                            tileNeighbour.Type = TileType::Tree;
                            tileNeighbour.TreeGrowth = 0.f;
                            tileNeighbour.TreeSpawnTimer = 0.f;
                        }
                    }
                }
            }

            // Check furnace
            if (tile.Type == TileType::Furnace)
            {
                tile.BurnTimer -= smoothDeltaTime;

                if (tile.BurnTimer <= 0.f)
                {
                    tile.BurnTimer = 0.f;

                    if (tile.Inventory->at(Items::Coal) > 0 && tile.Inventory->at(Items::IronOre) > 3)
                    {
                        tile.Inventory->at(Items::Coal)--;
                        tile.BurnTimer = 30.f;
                    }
                }

                if (tile.BurnTimer > 0.f)
                {
                    // Can melt some things
                    if (tile.SmeltTimer == 0.f)
                    {
                        // Check if he has enough items to smelt
                        if (tile.Inventory->at(Items::IronOre) > 3 && tile.Inventory->at(Items::IronIngot) < GetMaxItemsStored(tile, Items::IronIngot))
                        {
                            tile.Inventory->at(Items::IronOre) -= 3;
                            tile.SmeltTimer = 10.f;
                        }
                    }
                    else
                    {
                        tile.SmeltTimer -= smoothDeltaTime;

                        if (tile.SmeltTimer <= 0.f)
                        {
                            tile.SmeltTimer = 0.f;
                            tile.Inventory->at(Items::IronIngot)++;
                        }
                    }
                }
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
        (int)(position.X + _width / 2.f) / _tileSize,
        (int)(position.Y + _height / 2.f) / _tileSize};
}

[[nodiscard]] TilePosition Grid::GetTilePosition(int tileIndex) const
{
    return TilePosition{
        tileIndex % _width,
        tileIndex / _width};
}

Vector2F Grid::ToWorldPosition(TilePosition position) const
{
    return Vector2F{
        (float)position.X * _tileSize - _width / 2.f,
        (float)position.Y * _tileSize - _height / 2.f};
}

Tile &Grid::GetTile(TilePosition position)
{
    return _tiles[position.X + position.Y * _width];
}

Tile &Grid::GetTile(int index)
{
    return _tiles[index];
}

int Grid::GetTileIndex(TilePosition position) const
{
    return position.X + position.Y * _width;
}

bool Grid::IsTileValid(TilePosition position) const
{
    return position.X >= 0 && position.X < _width / _tileSize && position.Y >= 0 && position.Y < _height / _tileSize;
}

void Grid::SetTile(TilePosition position, Tile tile)
{
    // Set the tile as build if it's not a building
    if (tile.Type == TileType::None || tile.Type == TileType::Tree || tile.Type == TileType::Road || tile.Type == TileType::Stone)
    {
        tile.IsBuilt = true;
    }

    _tiles[position.X + position.Y * _width] = tile;
}

void Grid::RemoveTile(TilePosition position)
{
    _tiles[position.X + position.Y * _width] = Tile(TileType::None);
}

std::vector<TilePosition> Grid::GetTiles(TileType type) const
{
    std::vector<TilePosition> tiles;

    for (int x = 0; x < _width / _tileSize; x++)
    {
        for (int y = 0; y < _height / _tileSize; y++)
        {
            Tile &tile = _tiles[x + y * _width];

            if (tile.Type == type && tile.IsBuilt)
            {
                tiles.push_back(TilePosition{x, y});
            }
        }
    }

    return tiles;
}

std::vector<TilePosition> Grid::GetTiles(TileType type, TilePosition position, int radius) const
{
    std::vector<TilePosition> tiles;

    for (int x = position.X - radius; x <= position.X + radius; x++)
    {
        for (int y = position.Y - radius; y <= position.Y + radius; y++)
        {
            Tile &tile = _tiles[x + y * _width];

            if (tile.Type == type && tile.IsBuilt)
            {
                tiles.push_back(TilePosition{x, y});
            }
        }
    }

    return tiles;
}

std::vector<TilePosition> Grid::GetTiles(TilePosition position, int radius) const
{
    std::vector<TilePosition> tiles;

    for (int x = position.X - radius; x <= position.X + radius; x++)
    {
        for (int y = position.Y - radius; y <= position.Y + radius; y++)
        {
            Tile &tile = _tiles[x + y * _width];

            if (tile.Type != TileType::None && tile.IsBuilt)
            {
                tiles.push_back(TilePosition{x, y});
            }
        }
    }

    return tiles;
}

std::vector<TilePosition> Grid::GetTilesWithItems(TileType type, Items item) const
{
    std::vector<TilePosition> tiles;

    for (int x = 0; x < _width / _tileSize; x++)
    {
        for (int y = 0; y < _height / _tileSize; y++)
        {
            Tile &tile = _tiles[x + y * _width];

            if (tile.Type == type && tile.IsBuilt && tile.Inventory->at(item) > 0)
            {
                tiles.push_back(TilePosition{x, y});
            }
        }
    }

    return tiles;
}

void Grid::ForEachTile(const std::function<void(Tile &, TilePosition)> &callback) const
{
    for (int x = 0; x < _width / _tileSize; x++)
    {
        for (int y = 0; y < _height / _tileSize; y++)
        {
            Tile &tile = _tiles[x + y * _width];
            callback(tile, TilePosition{x, y});
        }
    }
}

float Grid::GetMaxConstructionProgress(TileType type)
{
    switch (type)
    {
        case TileType::Sawmill:
            return 10.f;
        case TileType::Quarry:
            return 15.f;
        case TileType::BuilderHut:
            return 10.f;
        case TileType::Storage:
            return 10.f;
        case TileType::House:
            return 30.f;
        case TileType::LogisticsCenter:
            return 10.f;
        case TileType::Furnace:
            return 15.f;
    }

    return 0.f;
}

float Grid::GetMaxDestructionProgress(TileType type)
{
    switch (type)
    {
        case TileType::Sawmill:
            return 5.f;
        case TileType::Quarry:
            return 7.5f;
        case TileType::BuilderHut:
            return 5.f;
        case TileType::Storage:
            return 5.f;
        case TileType::House:
            return 15.f;
        case TileType::Tree:
            return 5.f;
        case TileType::Stone:
            return 10.f;
        case TileType::LogisticsCenter:
            return 5.f;
        case TileType::Furnace:
            return 7.5f;
    }

    return 0.f;
}

float Grid::GetSpeedFactor(TileType type)
{
    switch (type)
    {
    case TileType::Quarry:
        return -0.5f;
    case TileType::BuilderHut:
        return 0.25f;
    case TileType::LogisticsCenter:
        return 0.5f;
    }

    return 0.f;
}

bool Grid::CanBuild(TilePosition position, TileType type)
{
    Tile &tile = GetTile(position);

    if (type == TileType::Quarry)
    {
        return tile.Type == TileType::Stone;
    }

    if (tile.Type == TileType::None)
    {
        return true;
    }

    return false;
}

bool Grid::CanBeDestroyed(TilePosition position)
{
    Tile &tile = GetTile(position);

    if (tile.Type == TileType::None) return false;
    if (tile.Type == TileType::LogisticsCenter && GetTiles(TileType::LogisticsCenter).size() == 1) return false;
    if (tile.Type == TileType::BuilderHut && GetTiles(TileType::BuilderHut).size() == 1) return false;
    if (tile.Type == TileType::MayorHouse) return false;
    if (!tile.IsBuilt) return false;

    return true;
}

int Grid::GetMaxItemsStored(const Tile &tile, Items item)
{
    if (!tile.IsBuilt || !tileMaxInventory->contains(tile.Type) || !tileMaxInventory->at(tile.Type).contains(item))
    {
        return 0;
    }

    return tileMaxInventory->at(tile.Type)[item];
}

int Grid::GetLeftSpaceForItems(Tile tile, Items item)
{
    int max = GetMaxItemsStored(tile, item);

    return max - tile.Inventory->at(item);
}

bool Grid::IsTileReadyToBuild(Tile &tile)
{
    // Check if the tile has the items to be built
    if (!tileNeededItems.contains(tile.Type)) return true;
    if (tileNeededItems[tile.Type].empty()) return true;

    for (auto &item : tileNeededItems[tile.Type])
    {
        if (tile.Inventory->at(item.first) < item.second)
        {
            return false;
        }
    }

    return true;
}

bool Grid::IsAStorage(TileType type)
{
    return type == TileType::Storage || type == TileType::LogisticsCenter || type == TileType::MayorHouse;
}

int Grid::GetNeededItemsToBuild(TileType type, Items item)
{
    if (!tileNeededItems.contains(type) || !tileNeededItems[type].contains(item))
    {
        return 0;
    }

    return tileNeededItems[type][item];
}

std::vector<TilePosition> Grid::GetPath(TilePosition start, TilePosition end)
{
    // Points per tile, the higher the value, the less the path will use this tile
    std::vector<int> pointsPerTile = std::vector<int>();

    for (int i = 0; i < (int)TileType::Count; i++)
    {
        pointsPerTile.push_back(20);
    }

    pointsPerTile[(int)TileType::None] = 10;
    pointsPerTile[(int)TileType::Road] = 1;

    // Search the full path between the start and the end tile by taking in account the pointsPerTile
    std::vector<TilePosition> path = std::vector<TilePosition>();

    std::vector<TilePosition> openList = std::vector<TilePosition>();
    std::vector<TilePosition> closedList = std::vector<TilePosition>();

    openList.push_back(start);

    // Use tile index
    auto cameFrom = std::vector<TilePosition>();

    auto gScore = std::vector<int>();
    auto fScore = std::vector<int>();

    for (int x = 0; x < _width / _tileSize; x++)
    {
        for (int y = 0; y < _height / _tileSize; y++)
        {
            cameFrom.push_back(TilePosition{-1, -1});
            gScore.push_back(0);
            fScore.push_back(0);
        }
    }

    gScore[start.X + start.Y * _width / _tileSize] = 0;
    fScore[start.X + start.Y * _width / _tileSize] = start.X - end.X + start.Y - end.Y;

    while (!openList.empty())
    {
        // Get the tile with the lowest fScore
        int lowestScore = INT_MAX;
        int lowestScoreIndex = -1;

        for (size_t i = 0; i < openList.size(); i++)
        {
            int index = openList[i].X + openList[i].Y * _width / _tileSize;

            if (fScore[index] < lowestScore)
            {
                lowestScore = fScore[index];
                lowestScoreIndex = i;
            }
        }

        TilePosition current = openList[lowestScoreIndex];

        if (current == end)
        {
            // Reconstruct the path
            TilePosition tmpCurrent = end;

            while (tmpCurrent != start)
            {
                path.push_back(tmpCurrent);
                tmpCurrent = cameFrom[tmpCurrent.X + tmpCurrent.Y * _width / _tileSize];
            }

            std::reverse(path.begin(), path.end());

            return path;
        }

        openList.erase(openList.begin() + lowestScoreIndex);
        closedList.push_back(current);

        for (auto &neighbour : GetNeighbours(current))
        {
            if (std::find(closedList.begin(), closedList.end(), neighbour) != closedList.end())
            {
                continue;
            }

            // The distance from start to a neighbor through current, the "distance between two adjacent tiles" is always 1
            int tentativeGScore = gScore[current.X + current.Y * _width / _tileSize] + pointsPerTile[(int)GetTile(neighbour).Type];

            if (std::find(openList.begin(), openList.end(), neighbour) == openList.end())
            {
                openList.push_back(neighbour);
            }
            else if (tentativeGScore >= gScore[neighbour.X + neighbour.Y * _width / _tileSize])
            {
                continue;
            }

            cameFrom[neighbour.X + neighbour.Y * _width / _tileSize] = current;
            gScore[neighbour.X + neighbour.Y * _width / _tileSize] = tentativeGScore;
            fScore[neighbour.X + neighbour.Y * _width / _tileSize] = gScore[neighbour.X + neighbour.Y * _width / _tileSize] + neighbour.X - end.X + neighbour.Y - end.Y;
        }
    }

    return path;
}

std::vector<TilePosition> Grid::GetNeighbours(TilePosition position) const
{
    std::vector<TilePosition> neighbours = std::vector<TilePosition>();

    if (position.X > 0)
    {
        neighbours.push_back(TilePosition{position.X - 1, position.Y});
    }

    if (position.X < _width / _tileSize - 1)
    {
        neighbours.push_back(TilePosition{position.X + 1, position.Y});
    }

    if (position.Y > 0)
    {
        neighbours.push_back(TilePosition{position.X, position.Y - 1});
    }

    if (position.Y < _height / _tileSize - 1)
    {
        neighbours.push_back(TilePosition{position.X, position.Y + 1});
    }

    return neighbours;
}

void Serialize(Serializer* ser, Grid* grid)
{
    Serialize(ser, &grid->_width);
    Serialize(ser, &grid->_height);
    Serialize(ser, &grid->_tileSize);

    for (int x = 0; x < grid->_width / grid->_tileSize; x++)
    {
        for (int y = 0; y < grid->_height / grid->_tileSize; y++)
        {
            //printf("current Tile : %i, %i \n", x, y); //Debug current Tile
            Serialize(ser, &grid->_tiles[x + y * grid->_width]);
        }
    }
}
