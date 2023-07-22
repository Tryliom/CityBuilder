#include "UnitManager.h"

#include <algorithm>
#include <thread>
#include "Graphics.h"
#include "Timer.h"
#include "Unit.h"
#include "Grid.h"
#include "Random.h"
#include "Logger.h"

float unitSpeed = 100.f;
int unitSize = 16;
float unitProgress;

std::map<TileType, std::map<Items, int>>* unitMaxInventory = new std::map<TileType, std::map<Items, int>>
{
	{TileType::Sawmill, {{Items::Wood, 30}}},
	{TileType::BuilderHut, {{Items::Wood, 30}, {Items::Stone, 20}}},
	{TileType::LogisticsCenter, {{Items::Wood, 50}, {Items::Stone, 50}, {Items::Coal, 25}, {Items::IronOre, 25}, {Items::IronIngot, 10}}},
};

void UnitManager::AddUnit(const Unit& unit)
{
	_units.push_back(unit);
}

void UnitManager::UpdateUnits()
{
	for (auto& unit : _units)
	{
        SendInactiveBuildersToBuild();

		if (unit.JobTileIndex != -1)
		{
			Tile& tile = _grid->GetTile(unit.JobTileIndex);

			if (tile.Type == TileType::None)
			{
				unit.JobTileIndex = -1;
				unit.SetBehavior(UnitBehavior::Idle);
				return;
			}

			unit.TimeSinceLastAction += Timer::SmoothDeltaTime;

			// Always the same for all units
			if (unit.CurrentBehavior == UnitBehavior::Moving)
			{
				auto targetPosition = _grid->ToWorldPosition(unit.TargetTile);

				if (targetPosition == unit.Position)
				{
					unit.SetBehavior(UnitBehavior::Working);
				}
				else
				{
					if (!unit.CalculatingPath)
					{
						// Start a new thread to calculate the path
						unit.CalculatingPath = true;
						std::thread([&]()
			            {
							auto list = _grid->GetPath(_grid->GetTilePosition(unit.Position), unit.TargetTile);

							if (list.empty())
							{
								unit.SetBehavior(UnitBehavior::Working);
							}
							else
							{
                                unit.PathToTargetTile = list;
							}
			            }).detach();
					}

					if (!unit.PathToTargetTile.empty())
					{
						auto nextPosition = GetNextUnitPosition(unit);
						auto nextTileWorldPosition = GetNextTargetPosition(unit);

                        float distance = nextTileWorldPosition.GetDistance(nextPosition);
                        float previousDistance = nextTileWorldPosition.GetDistance(unit.Position);
                        bool hasReached = previousDistance < distance || nextPosition == nextTileWorldPosition;

						// Check if it reached the center of the next tile or if it's too far
						if (hasReached)
						{
							unit.PathToTargetTile.erase(unit.PathToTargetTile.begin());

							// Check if it's the last tile
							if (unit.PathToTargetTile.empty())
							{
								unit.SetBehavior(UnitBehavior::Working);
							}
							else
							{
								unit.Position = GetNextUnitPosition(unit);
								unit.CalculatingPath = false;
							}
						}
						else
						{
							unit.Position = GetNextUnitPosition(unit);
						}
					}
				}
			}

            auto lastBehavior = unit.CurrentBehavior;

			if (tile.Type == TileType::Sawmill) OnTickUnitSawMill(unit);
			else if (tile.Type == TileType::BuilderHut) OnTickUnitBuilderHut(unit);
			else if (tile.Type == TileType::LogisticsCenter) onTickUnitLogistician(unit);
			else if (tile.Type == TileType::Quarry) OnTickUnitQuarry(unit);

            // Make them move to their job tile if they have nothing to do
            auto jobPosition = _grid->GetTilePosition(unit.JobTileIndex);

            if (lastBehavior == UnitBehavior::Idle && lastBehavior == unit.CurrentBehavior && _grid->GetTilePosition(unit.Position) != jobPosition)
            {
                unit.TargetTile = jobPosition;
                unit.SetBehavior(UnitBehavior::Moving);
            }
		}
		else
		{
			// Try to get a job, priority to the tile with the lowest amount of workers or 0
			auto jobs = GetAvailableJobs();

			if (jobs.empty()) continue;

			std::sort(jobs.begin(), jobs.end(), [&](int a, int b)
			{
				int aWorkers = CountHowManyUnitAreWorkingOn(a);
				int bWorkers = CountHowManyUnitAreWorkingOn(b);

				return aWorkers < bWorkers;
			});

			unit.JobTileIndex = jobs[0];
		}

		// Remove the overflow of items
		if (unit.JobTileIndex == -1) continue;

		for (auto& item: *unit.Inventory)
		{
			int max = GetMaxItemsFor(unit, item.first);

			item.second = std::min(item.second, max);
		}
	}

	// Check if there is enough place for a new unit
	size_t housesCount = _grid->GetTiles(TileType::House).size();

	if (_units.size() < housesCount * 5)
	{
		unitProgress += Timer::SmoothDeltaTime;

		if (unitProgress > 20.f)
		{
			unitProgress = 0.f;

            // Spawn a new unit per house with 20% chance and still enough place
            for (auto& house : _grid->GetTiles(TileType::House))
            {
                if (Random::Range(0, 100) < 20 && _units.size() < housesCount * 5)
                {
                    AddUnit(Unit(_grid->ToWorldPosition(house) + Vector2F(0.5f, 1.f) * (float) (_grid->GetTileSize() - unitSize)));
                }
            }
		}
	}
}

void UnitManager::OnTickUnitSawMill(Unit& unit)
{
	Tile& jobTile = _grid->GetTile(unit.JobTileIndex);

	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		// Check if the unit need to drop items at the sawmill
		if (NeedToDropItemsAtJob(unit, Items::Wood, InventoryReason::MoreThanHalf))
		{
			// Go back to the sawmill to drop the logs
			unit.TargetTile = _grid->GetTilePosition(unit.JobTileIndex);
			unit.SetBehavior(UnitBehavior::Moving);
			return;
		}

		// Check if there is a full tree
		auto treePositions = GetAllHarvestableTrees(_grid->GetTilePosition(unit.JobTileIndex), 3);

		if (!treePositions.empty())
		{
			// Go to the tree
			unit.TargetTile = treePositions[0];
			unit.SetBehavior(UnitBehavior::Moving);
			return;
		}

		// If the unit has nothing to do, check if he has wood in his inventory, then go to the sawmill drop it
		if (NeedToDropItemsAtJob(unit, Items::Wood, InventoryReason::MoreThanOne))
		{
			unit.TargetTile = _grid->GetTilePosition(unit.JobTileIndex);
			unit.SetBehavior(UnitBehavior::Moving);
			return;
		}
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		Tile& tile = _grid->GetTile(unit.TargetTile);

		// Drop the logs in the sawmill
		if (tile.Type == TileType::Sawmill)
		{
			// Drop the logs in the sawmill
			int spaceLeft = Grid::GetLeftSpaceForItems(jobTile, Items::Wood);
			int logsToDrop = std::min(unit.Inventory->at(Items::Wood), spaceLeft);

			unit.Inventory->at(Items::Wood) -= logsToDrop;
			jobTile.Inventory->at(Items::Wood) += logsToDrop;

			unit.SetBehavior(UnitBehavior::Idle);
		}
		// Harvest the tree
		else if (unit.TimeSinceLastAction > 2.f)
		{
			tile.TreeGrowth = 0.f;

			unit.Inventory->at(Items::Wood) += 5;

			unit.SetBehavior(UnitBehavior::Idle);
		}
	}
}

void UnitManager::OnTickUnitBuilderHut(Unit& unit)
{
	if (unit.CurrentBehavior == UnitBehavior::Moving)
	{
		Tile& tile = _grid->GetTile(unit.TargetTile);

		// Check that the tile is still valid
		if (!Grid::IsAStorage(tile.Type) && unit.TargetTile != _grid->GetTilePosition(unit.JobTileIndex) && (tile.IsBuilt && !tile.NeedToBeDestroyed) || tile.Type == TileType::None)
		{
			unit.SetBehavior(UnitBehavior::Idle);
		}
	}

	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
        auto searchAStorage = [&]()
        {
            for (auto pair : *unit.Inventory)
            {
                if (pair.second == 0) continue;

                auto storagePositions = GetStorageAroundFor(_grid->GetTilePosition(unit.Position), pair.first);

                if (storagePositions.empty()) continue;

                unit.TargetTile = storagePositions[0];
                unit.SetBehavior(UnitBehavior::Moving);

                return true;
            }

            return false;
        };

		// Check if his inventory is more than half full
		if (IsInventoryHalfFull(unit))
		{
			// Search for a storage free space to drop the resources he has around his builder house
            if (searchAStorage()) return;
		}

		// Check if there is a construction to build
		auto workPositions = GetAllBuildableOrDestroyableTiles();

		if (!workPositions.empty())
		{
            // Sort it to have the closest one first
            std::sort(workPositions.begin(), workPositions.end(), [&](TilePosition a, TilePosition b)
            {
                return _grid->GetTilePosition(unit.Position).GetDistance(a) < _grid->GetTilePosition(unit.Position).GetDistance(b);
            });

			unit.TargetTile = workPositions[0];
			unit.SetBehavior(UnitBehavior::Moving);
			return;
		}

        // Check if he has something in his inventory
        if (!IsInventoryEmpty(unit))
        {
            // Search for a storage free space to drop the resources he has around his builder house
            if (searchAStorage()) return;
        }
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		Tile& tile = _grid->GetTile(unit.TargetTile);

		if (!tile.NeedToBeDestroyed && tile.IsBuilt)
		{
			if (Grid::IsAStorage(tile.Type))
			{
				// Drop all the resources in the storage
				for (auto pair : *unit.Inventory)
				{
					int spaceLeft = Grid::GetLeftSpaceForItems(tile, pair.first);
					int itemsToDrop = std::min(pair.second, spaceLeft);

					unit.Inventory->at(pair.first) -= itemsToDrop;
					tile.Inventory->at(pair.first) += itemsToDrop;
				}
			}

			unit.SetBehavior(UnitBehavior::Idle);
			return;
		}

		tile.Progress += Timer::SmoothDeltaTime;

		// Build the tile
		if (!tile.IsBuilt && tile.Progress >= Grid::GetMaxConstructionProgress(tile.Type))
		{
			tile.IsBuilt = true;
			tile.NeedToBeDestroyed = false;
			tile.Progress = 0.f;

			// Remove all the resources from the inventory of the tile that was used to build the tile
			for (auto pair : *tile.Inventory)
			{
				tile.Inventory->at(pair.first) -= Grid::GetNeededItemsToBuild(tile.Type, pair.first);
			}

			unit.SetBehavior(UnitBehavior::Idle);
		}
		// Destroy the tile
		else if (tile.NeedToBeDestroyed && tile.Progress >= Grid::GetMaxDestructionProgress(tile.Type))
		{
			if (tile.Type == TileType::Tree)
			{
				unit.Inventory->at(Items::Wood) += 5;
			}
			else if (tile.Type == TileType::Stone)
			{
				unit.Inventory->at(Items::Stone) += 20;
			}

            // Builder receive all the resources from the tile
            for (auto pair : *tile.Inventory)
            {
                unit.Inventory->at(pair.first) += pair.second;
            }

			tile.Reset();
			unit.SetBehavior(UnitBehavior::Idle);
		}
	}
}

void UnitManager::onTickUnitLogistician(Unit& unit)
{
	if (unit.CurrentBehavior == UnitBehavior::Moving)
	{
		Tile& tile = _grid->GetTile(unit.TargetTile);

		// Check that the tile is still valid
		if (tile.Type == TileType::None)
		{
			unit.SetBehavior(UnitBehavior::Idle);
		}
	}

	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		// Check if there is a construction to build that need resources
		auto buildsThatNeedResources = GetTilesThatNeedItemsToBeBuilt();

		if (!buildsThatNeedResources.empty())
		{
			auto tilePosition = buildsThatNeedResources[0];

			// Check if the unit has the resources to build it or need to go to a storage to get them
			for (auto pair : *_grid->GetTile(tilePosition).Inventory)
			{
				Items item = pair.first;
				int quantity = pair.second;
				int neededItems = Grid::GetNeededItemsToBuild(_grid->GetTile(tilePosition).Type, item);

				if (quantity >= neededItems) continue;

				int itemsToGet = neededItems - quantity;
                int unitItem = unit.Inventory->at(item);

				// Check if the unit has the resources in his inventory or has his inventory full of this item
				if (unitItem >= itemsToGet || unitItem == GetMaxItemsFor(unit, item)) continue;

				// Search for a storage that has the resources
				auto storagePositions = GetStorageThatHave(_grid->GetTilePosition(unit.Position), item);

				if (storagePositions.empty()) continue;

				unit.TargetTile = storagePositions[0];
				unit.SetBehavior(UnitBehavior::Moving);
				return;
			}

            // Check if the unit has more than 0 of the items needed for the construction
			if (HasAtLeastOneItemNeededToBuild(unit, tilePosition))
			{
				unit.TargetTile = tilePosition;
				unit.SetBehavior(UnitBehavior::Moving);
				return;
			}
		}

        // Check if there is a furnace that need coal or/and iron ore
        auto furnaces = GetFurnacesThatNeedItems();

        for (auto tilePosition : furnaces)
        {
            Tile& tile = _grid->GetTile(tilePosition);
            Items itemToGet = Items::Coal;

            if (tile.Inventory->at(Items::Coal) >= Grid::GetMaxItemsStored(tile, Items::Coal) / 2)
            {
                itemToGet = Items::IronOre;
            }

            if (unit.Inventory->at(itemToGet) > 0)
            {
                // Go to the furnace to drop the items
                unit.TargetTile = tilePosition;
                unit.SetBehavior(UnitBehavior::Moving);
                return;
            }
            else
            {
                // Search a storage
                auto storagePositions = GetStorageThatHave(_grid->GetTilePosition(unit.Position), itemToGet);

                if (storagePositions.empty()) continue;

                unit.TargetTile = storagePositions[0];
                unit.SetBehavior(UnitBehavior::Moving);
                return;
            }
        }

        if (!IsInventoryEmpty(unit))
        {
            // Search for a storage free space to drop the resources he has
            for (auto pair : *unit.Inventory)
            {
                if (pair.second == 0) continue;

                auto storagePositions = GetStorageAroundFor(_grid->GetTilePosition(unit.Position), pair.first);

                if (storagePositions.empty()) continue;

                unit.TargetTile = storagePositions[0];
                unit.SetBehavior(UnitBehavior::Moving);
                return;
            }
        }

		std::vector<TilePosition> tilesToGetItemsFrom = {};
		auto sawmills = _grid->GetTiles(TileType::Sawmill);
		auto quarries = _grid->GetTiles(TileType::Quarry);
        auto furnaceToGet = _grid->GetTiles(TileType::Furnace);
        auto addList = [&](const std::vector<TilePosition>& list)
        {
            for (auto tile : list)
            {
                Tile& tileRef = _grid->GetTile(tile);

                if (IsTileTakenCareBy(tile, Characters::Logistician) || tileRef.GetInventorySize() == 0) continue;

                // Check if any of the items in the tile can be pickup by the unit
                for (auto pair : *_grid->GetTile(tile).Inventory)
                {
                    if (pair.second == 0) continue;
                    if (tileRef.Type == TileType::Furnace && pair.first != Items::IronIngot) continue;

                    if (unit.Inventory->at(pair.first) < GetMaxItemsFor(unit, pair.first))
                    {
                        tilesToGetItemsFrom.push_back(tile);
                        break;
                    }
                }
            }
        };

		tilesToGetItemsFrom.reserve(sawmills.size() + quarries.size());

        addList(sawmills);
        addList(quarries);
        addList(furnaceToGet);

		std::sort(tilesToGetItemsFrom.begin(), tilesToGetItemsFrom.end(), [&](TilePosition a, TilePosition b)
		{
			return _grid->GetTile(a).GetInventorySize() > _grid->GetTile(b).GetInventorySize();
		});

		if (!tilesToGetItemsFrom.empty())
		{
			unit.TargetTile = tilesToGetItemsFrom[0];
			unit.SetBehavior(UnitBehavior::Moving);
			return;
		}
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		unit.TimeSinceLastAction += Timer::SmoothDeltaTime;

		if (unit.TimeSinceLastAction < 1.f) return;

		Tile& tile = _grid->GetTile(unit.TargetTile);

		// If it's a building that is not built, add the needed resources to it
		if (!tile.IsBuilt)
		{
			for (auto pair : *tile.Inventory)
			{
				int neededItems = Grid::GetNeededItemsToBuild(tile.Type, pair.first);

				if (pair.second >= neededItems) continue;

				int itemsToGet = neededItems - pair.second;
				int itemToDrop = std::min(unit.Inventory->at(pair.first), itemsToGet);

				unit.Inventory->at(pair.first) -= itemToDrop;
				tile.Inventory->at(pair.first) += itemToDrop;
			}
		}
		// If it's a storage, first check if there is something to build that need resources
		else if (Grid::IsAStorage(tile.Type))
		{
			auto buildsThatNeedResources = GetTilesThatNeedItemsToBeBuilt();
            auto furnaces = GetFurnacesThatNeedItems();

			if (!buildsThatNeedResources.empty())
			{
				auto tilePosition = buildsThatNeedResources[0];
                Tile& buildTile = _grid->GetTile(tilePosition);

				// Check if the unit has the resources to build it or need to go to a storage to get them
				for (auto pair : *buildTile.Inventory)
				{
					Items item = pair.first;
					int quantity = pair.second;
					int neededItems = Grid::GetNeededItemsToBuild(buildTile.Type, item);

					if (quantity >= neededItems) continue;

					int itemsToGet = neededItems - quantity;
                    int tileItem = tile.Inventory->at(item);
                    int unitItem = unit.Inventory->at(item);

					// Check if the unit has the resources in his inventory or has his inventory full of this item
					if (unitItem >= itemsToGet || unitItem == GetMaxItemsFor(unit, item)) continue;

                    itemsToGet = std::min(itemsToGet, tileItem);

					// Get the resources from the storage
                    tile.Inventory->at(item) -= itemsToGet;
                    unit.Inventory->at(item) += itemsToGet;
				}
			}
            else if (!furnaces.empty() && (tile.Inventory->at(Items::Coal) > 0 || tile.Inventory->at(Items::IronOre) > 0))
            {
                auto furnaceTile = _grid->GetTile(furnaces[0]);

                // Try to get the items from the storage (coal and iron ore)
                for (auto pair : *tile.Inventory)
                {
                    if (pair.second == 0 || (pair.first != Items::Coal && pair.first != Items::IronOre)) continue;

                    Items item = pair.first;
                    int quantity = pair.second;
                    int neededItems = Grid::GetMaxItemsStored(furnaceTile, item) - furnaceTile.Inventory->at(item);
                    int unitItem = unit.Inventory->at(item);

                    // Check if the unit has the resources in his inventory or has his inventory full of this item
                    if (unitItem >= neededItems || unitItem == GetMaxItemsFor(unit, item)) continue;

                    int itemsToGet = std::min(GetMaxItemsFor(unit, item) - unitItem, std::min(quantity, neededItems));

                    // Get the resources from the storage
                    tile.Inventory->at(item) -= itemsToGet;
                    unit.Inventory->at(item) += itemsToGet;
                    break;
                }
            }
			// Check if there is resources to move to the storage from the unit
			else
			{
				for (auto pair : *unit.Inventory)
				{
					if (pair.second == 0) continue;

					int itemsToDrop = pair.second;
					int spaceLeft = Grid::GetLeftSpaceForItems(tile, pair.first);
					int itemsToDropInStorage = std::min(itemsToDrop, spaceLeft);

					// Drop the items
					unit.Inventory->at(pair.first) -= itemsToDropInStorage;
					tile.Inventory->at(pair.first) += itemsToDropInStorage;
				}
			}
		}
		// If it's a sawmill or a quarry, check if there is resources to get from it
		else if (tile.Type == TileType::Sawmill || tile.Type == TileType::Quarry)
		{
			for (auto pair : *tile.Inventory)
			{
				if (pair.second == 0) continue;

				int itemsToDrop = pair.second;
				int spaceLeftInUnit = GetMaxItemsFor(unit, pair.first) - unit.Inventory->at(pair.first);
				int itemsToDropInUnit = std::min(itemsToDrop, spaceLeftInUnit);

				// Drop the items
				unit.Inventory->at(pair.first) += itemsToDropInUnit;
				tile.Inventory->at(pair.first) -= itemsToDropInUnit;
			}
		}
        // If it's a furnace, check if there is coal or iron ore to drop on it and iron ingots to get
        else if (tile.Type == TileType::Furnace)
        {
            int coalToDrop = std::min(unit.Inventory->at(Items::Coal), Grid::GetMaxItemsStored(tile, Items::Coal) - tile.Inventory->at(Items::Coal));
            int ironOreToDrop = std::min(unit.Inventory->at(Items::IronOre), Grid::GetMaxItemsStored(tile, Items::IronOre) - tile.Inventory->at(Items::IronOre));

            unit.Inventory->at(Items::Coal) -= coalToDrop;
            unit.Inventory->at(Items::IronOre) -= ironOreToDrop;

            tile.Inventory->at(Items::Coal) += coalToDrop;
            tile.Inventory->at(Items::IronOre) += ironOreToDrop;

            // Check if there is iron ingots to get
            if (tile.Inventory->at(Items::IronIngot) > 0)
            {
                int ingotsToDrop = std::min(tile.Inventory->at(Items::IronIngot), GetMaxItemsFor(unit, Items::IronIngot) - unit.Inventory->at(Items::IronIngot));

                tile.Inventory->at(Items::IronIngot) -= ingotsToDrop;
                unit.Inventory->at(Items::IronIngot) += ingotsToDrop;
            }
        }

		unit.SetBehavior(UnitBehavior::Idle);
	}
}

void UnitManager::OnTickUnitQuarry(Unit& unit)
{
	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		TilePosition tilePosition = _grid->GetTilePosition(unit.JobTileIndex);

		// Check if the quarry is full
		if (Grid::GetLeftSpaceForItems(_grid->GetTile(tilePosition), Items::Stone) == 0) return;

		// If the unit is not on the quarry, move to it
		if (tilePosition != unit.TargetTile)
		{
			unit.TargetTile = tilePosition;
			unit.SetBehavior(UnitBehavior::Moving);
		}
		else
		{
			unit.SetBehavior(UnitBehavior::Working);
		}
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		if (unit.TimeSinceLastAction < 5.f) return;

		Tile& tile = _grid->GetTile(unit.TargetTile);

		int stoneLeftSpace = Grid::GetLeftSpaceForItems(tile, Items::Stone);
		int coalLeftSpace = Grid::GetLeftSpaceForItems(tile, Items::Coal);
		int ironOreLeftSpace = Grid::GetLeftSpaceForItems(tile, Items::IronOre);

		// Check if the quarry is full
		if (stoneLeftSpace != 0 || coalLeftSpace != 0 || ironOreLeftSpace != 0)
		{
			auto rand = Random::Range(0, 100);

			if (rand < 5 && ironOreLeftSpace != 0)
			{
				tile.Inventory->at(Items::IronOre) += Random::Range(1, 5);
			}
			else if (rand < 10 && coalLeftSpace != 0)
			{
				tile.Inventory->at(Items::Coal) += Random::Range(1, 3);
			}
			else if (stoneLeftSpace != 0)
			{
				tile.Inventory->at(Items::Stone) += 1;
			}
		}

		unit.SetBehavior(UnitBehavior::Idle);
	}
}

void UnitManager::SendInactiveBuildersToBuild()
{
    auto inactiveBuilders = GetAllInactive(Characters::Builder);
    auto buildableTiles = GetAllBuildableOrDestroyableTiles();

    if (!inactiveBuilders.empty() && !buildableTiles.empty())
    {
        std::vector<int> buildersToCallFirst = {};

        // For each builder, check the closest buildable tile and check which one is the closest
        for (auto builder: inactiveBuilders)
        {
            bool alreadyCalled = false;

            for (int called: buildersToCallFirst)
            {
                if (called == builder) alreadyCalled = true;
            }

            if (alreadyCalled) continue;

            Unit& builderUnit = _units[builder];

            std::sort(buildableTiles.begin(), buildableTiles.end(), [&](TilePosition a, TilePosition b)
            {
                return _grid->GetTilePosition(builderUnit.Position).GetDistance(a) < _grid->GetTilePosition(builderUnit.Position).GetDistance(b);
            });

            // Check for each other builders that they don't have a shortest distance to the tile
            int bestBuilder = builder;
            float bestDistance = _grid->GetTilePosition(builderUnit.Position).GetDistance(buildableTiles[0]);

            for (auto otherBuilder: inactiveBuilders)
            {
                if (otherBuilder == builder) continue;

                Unit& otherUnit = _units[otherBuilder];

                float distance = _grid->GetTilePosition(otherUnit.Position).GetDistance(buildableTiles[0]);

                if (distance < bestDistance)
                {
                    bestBuilder = otherBuilder;
                    bestDistance = distance;
                }
            }

            if (bestBuilder == builder)
            {
                buildersToCallFirst.push_back(builder);
            }
        }

        // Call the builders that are the closest to the buildable tiles
        for (auto builder: buildersToCallFirst)
        {
            OnTickUnitBuilderHut(_units[builder]);
        }
    }
}

Vector2F UnitManager::GetNextUnitPosition(Unit& unit)
{
	Tile& tile = _grid->GetTile(unit.JobTileIndex);
	Vector2F nextTileWorldPosition = GetNextTargetPosition(unit);
	float speedFactor = 1.f;

	// Check if the next tile is a road
	if (_grid->GetTile(_grid->GetTilePosition(unit.Position)).Type == TileType::Road)
	{
		speedFactor = 1.5f;
	}

	speedFactor += Grid::GetSpeedFactor(tile.Type);

	// Move it to the center of the next tile
	auto offset = (nextTileWorldPosition - unit.Position).Normalized() * unitSpeed * speedFactor * Timer::SmoothDeltaTime;

	return unit.Position + offset;
}

Vector2F UnitManager::GetNextTargetPosition(Unit& unit)
{
	Vector2F nextTileWorldPosition = _grid->ToWorldPosition(unit.PathToTargetTile[0]) + Vector2F(0.5f, 0.5f) * (float) (_grid->GetTileSize() - unitSize);

	// Check if it's the last tile to set the target position to the center-bottom of the tile
	if (unit.PathToTargetTile.size() == 1)
	{
		nextTileWorldPosition += Vector2F(0.f, 1.f) * ((float) _grid->GetTileSize()) / 2.f;
	}

	return nextTileWorldPosition;
}

void UnitManager::DrawUnits(bool drawBehindBuildings)
{
	for (auto& unit : _units)
	{
		Characters character = GetCharacter(unit.JobTileIndex);
		TilePosition tilePosition = _grid->GetTilePosition(unit.Position);
		// Check if the character is positioned before 80% of the height of the tile
		bool isBehindBuilding = tilePosition == _grid->GetTilePosition(unit.Position + Vector2F(0.f, _grid->GetTileSize() * 0.21f));

		if (drawBehindBuildings != isBehindBuilding) continue;

		Graphics::DrawObject({
            .Position = unit.Position,
            .Size = {unitSize, unitSize},
            .Texture = Texture(character),
        });
	}
}

Characters UnitManager::GetCharacter(int jobTileIndex)
{
	if (jobTileIndex == -1) return Characters::Unemployed;

	Tile& jobTile = _grid->GetTile(jobTileIndex);

	if (jobTile.Type == TileType::Sawmill)
	{
		return Characters::Lumberjack;
	}
	else if (jobTile.Type == TileType::BuilderHut)
	{
		return Characters::Builder;
	}
	else if (jobTile.Type == TileType::Quarry)
	{
		return Characters::Digger;
	}
	else if (jobTile.Type == TileType::LogisticsCenter)
	{
		return Characters::Logistician;
	}

	return Characters::Unemployed;
}

bool UnitManager::IsTileTakenCareBy(TilePosition position, Characters character)
{
	for (auto& unit : _units)
	{
		if (unit.JobTileIndex == -1) continue;

		if ((unit.CurrentBehavior == UnitBehavior::Moving || unit.CurrentBehavior == UnitBehavior::Working) && unit.TargetTile == position && GetCharacter(unit.JobTileIndex) == character)
		{
			return true;
		}
	}

	return false;
}

bool UnitManager::IsTileJobFull(int jobTileIndex)
{
	Tile& tile = _grid->GetTile(jobTileIndex);

	if (!tile.IsBuilt) return true;

	return CountHowManyUnitAreWorkingOn(jobTileIndex) >= GetMaxUnitOnJob(jobTileIndex);
}

int UnitManager::GetMaxUnitOnJob(int jobTileIndex)
{
	Tile& tile = _grid->GetTile(jobTileIndex);

	if (!tile.IsBuilt) return true;

	switch (tile.Type)
	{
		case TileType::Sawmill: return 2;
		case TileType::BuilderHut: return 1;
		case TileType::Quarry: return 2;
		case TileType::LogisticsCenter: return 1;

		default: return 0;
	}
}

int UnitManager::CountHowManyUnitAreWorkingOn(int jobTileIndex)
{
	int result = 0;

	for (auto& unit : _units)
	{
		if (unit.JobTileIndex == jobTileIndex) result++;
	}

	return result;
}

std::vector<int> UnitManager::GetAllInactive(Characters character)
{
    std::vector<int> result = std::vector<int>();
    int index = 0;

    for (auto& unit : _units)
    {
        if (unit.JobTileIndex != -1 && GetCharacter(unit.JobTileIndex) == character && unit.CurrentBehavior == UnitBehavior::Idle)
        {
            result.push_back(index);
        }

        index++;
    }

    return result;
}

std::vector<TilePosition> UnitManager::GetAllHarvestableTrees(TilePosition position, int radius)
{
	std::vector<TilePosition> trees = std::vector<TilePosition>();
	auto treePositions = _grid->GetTiles(TileType::Tree, position, radius);

	for (auto& treePosition : treePositions)
	{
		auto& treeTile = _grid->GetTile(treePosition);

		if (treeTile.TreeGrowth < 30.f || IsTileTakenCareBy(treePosition, Characters::Lumberjack)) continue;

		trees.push_back(treePosition);
	}

	return trees;
}

bool UnitManager::NeedToDropItemsAtJob(Unit& unit, Items item, InventoryReason reason)
{
	Tile& jobTile = _grid->GetTile(unit.JobTileIndex);
	int maxItems = GetMaxItemsFor(unit, item);

	if (jobTile.Inventory->at(item) == Grid::GetMaxItemsStored(jobTile, item)) return false;
	if (reason == InventoryReason::Full && unit.Inventory->at(item) < maxItems) return false;
	if (reason == InventoryReason::MoreThanHalf && unit.Inventory->at(item) < maxItems / 2) return false;
	if (reason == InventoryReason::MoreThanOne && unit.Inventory->at(item) == 0) return false;

	return true;
}

std::vector<TilePosition> UnitManager::GetStorageAroundFor(TilePosition position, Items item)
{
	std::vector<TilePosition> storages = std::vector<TilePosition>();

    _grid->ForEachTile([&](Tile& tile, TilePosition position)
    {
        if (!Grid::IsAStorage(tile.Type) || !tile.IsBuilt || tile.NeedToBeDestroyed) return;
        if (tile.Inventory->at(item) == Grid::GetMaxItemsStored(tile, item)) return;

        storages.push_back(position);
    });

    // Sort them by the less distance
    std::sort(storages.begin(), storages.end(), [&](TilePosition a, TilePosition b)
    {
        return std::abs(position.GetDistance(a)) < std::abs(position.GetDistance(b));
    });

	return storages;
}

std::vector<TilePosition> UnitManager::GetAllBuildableOrDestroyableTiles()
{
	std::vector<TilePosition> tiles = std::vector<TilePosition>();

	_grid->ForEachTile([&](Tile& tile, TilePosition position)
	{
		if (IsTileTakenCareBy(position, Characters::Builder) || tile.Type == TileType::None) return;

		if ((!tile.IsBuilt && Grid::IsTileReadyToBuild(tile)) || (tile.IsBuilt && tile.NeedToBeDestroyed))
		{
			tiles.push_back(position);
		}
	});

	return tiles;
}

std::vector<TilePosition> UnitManager::GetTilesThatNeedItemsToBeBuilt()
{
	std::vector<TilePosition> tiles = std::vector<TilePosition>();
	auto items = GetAllUsableItems();

	_grid->ForEachTile([&](Tile& tile, TilePosition position)
    {
        if (IsTileTakenCareBy(position, Characters::Logistician) || tile.Type == TileType::None) return;
        if (Grid::IsTileReadyToBuild(tile) || tile.IsBuilt) return;

        // Check if we have the items to build the tile
        bool canBuild = true;

        for (auto& item : items)
        {
            if (Grid::GetNeededItemsToBuild(tile.Type, item.first) - tile.Inventory->at(item.first) > item.second)
            {
                canBuild = false;
                break;
            }
        }

        if (!canBuild) return;

        tiles.push_back(position);
    });

	return tiles;
}

std::vector<TilePosition> UnitManager::GetStorageThatHave(TilePosition position, Items item)
{
	std::vector<TilePosition> storages = std::vector<TilePosition>();

	_grid->ForEachTile([&](Tile& tile, TilePosition position)
	{
        if (tile.Type == TileType::None || !tile.IsBuilt || tile.NeedToBeDestroyed) return;
		if (!Grid::IsAStorage(tile.Type) && tile.Type != TileType::Sawmill && tile.Type != TileType::Quarry) return;
		if (tile.Inventory->at(item) == 0) return;

		storages.push_back(position);
	});

    // Sort them by the less distance
    std::sort(storages.begin(), storages.end(), [&](TilePosition a, TilePosition b)
    {
        return std::abs(position.GetDistance(a)) < std::abs(position.GetDistance(b));
    });

	return storages;
}

std::vector<int> UnitManager::GetAvailableJobs()
{
	std::vector<int> jobs = std::vector<int>();

	_grid->ForEachTile([&](Tile& tile, TilePosition position)
	{
		int index = _grid->GetTileIndex(position);

		if (tile.Type == TileType::None) return;
		if (IsTileJobFull(index)) return;

		jobs.push_back(index);
	});

	return jobs;
}

std::vector<TilePosition> UnitManager::GetFurnacesThatNeedItems()
{
    std::vector<TilePosition> furnaces = std::vector<TilePosition>();

    _grid->ForEachTile([&](Tile& tile, TilePosition position)
    {
        if (tile.Type != TileType::Furnace || !tile.IsBuilt || tile.NeedToBeDestroyed) return;

        if (tile.Inventory->at(Items::Coal) < Grid::GetMaxItemsStored(tile, Items::Coal) ||
            tile.Inventory->at(Items::IronOre) < Grid::GetMaxItemsStored(tile, Items::IronOre))
        {
            furnaces.push_back(position);
        }
    });

    // Sort the furnaces by the one that have the less coal and iron ore
    std::sort(furnaces.begin(), furnaces.end(), [&](TilePosition a, TilePosition b)
    {
        Tile& tileA = _grid->GetTile(a);
        Tile& tileB = _grid->GetTile(b);
        int aItems = tileA.Inventory->at(Items::Coal) + tileA.Inventory->at(Items::IronOre);
        int bItems = tileB.Inventory->at(Items::Coal) + tileB.Inventory->at(Items::IronOre);

        return aItems < bItems;
    });

    return furnaces;
}

int UnitManager::GetMaxItemsFor(Unit& unit, Items item)
{
	if (unit.JobTileIndex == -1) return 0;

	Tile& tile = _grid->GetTile(unit.JobTileIndex);

	if (!tile.IsBuilt) return 0;

	if (!unitMaxInventory->contains(tile.Type) || !unitMaxInventory->at(tile.Type).contains(item))
	{
		return 0;
	}

	return unitMaxInventory->at(tile.Type).at(item);
}

bool UnitManager::IsInventoryEmpty(Unit& unit)
{
	for (auto& item : *unit.Inventory)
	{
		if (item.second > 0) return false;
	}

	return true;
}

bool UnitManager::IsInventoryHalfFull(Unit& unit)
{
    for (auto& item : *unit.Inventory)
    {
        if (item.second > GetMaxItemsFor(unit, item.first) / 2)
        {
            return true;
        }
    }

    return false;
}

std::map<Items, int> UnitManager::GetAllUsableItems()
{
	std::map<Items, int> items = std::map<Items, int>();

	for (int i = 0; i < (int) Items::Count; i++)
	{
		items.insert(std::pair<Items, int>((Items) i, 0));
	}

	// Add all items from logisticians
	for (auto& unit : _units)
	{
		if (unit.JobTileIndex == -1) continue;

		auto& tile = _grid->GetTile(unit.JobTileIndex);

		if (tile.Type != TileType::LogisticsCenter) continue;

		for (auto& item : *unit.Inventory)
		{
			items.at(item.first) += item.second;
		}
	}

    auto isValidToCountItemsFromIt = [&](TileType type)
    {
        return Grid::IsAStorage(type) || type == TileType::Sawmill || type == TileType::Quarry;
    };

	// Add all items from storages
	_grid->ForEachTile([&](Tile& tile, TilePosition position)
	{
		if (!tile.IsBuilt || tile.NeedToBeDestroyed || !isValidToCountItemsFromIt(tile.Type)) return;

		for (auto& item : *tile.Inventory)
		{
			items.at(item.first) += item.second;
		}
	});

	return items;
}

bool UnitManager::HasAtLeastOneItemNeededToBuild(Unit& unit, TilePosition position)
{
    auto& tile = _grid->GetTile(position);

    if (tile.Type == TileType::None) return false;

    for (auto& item : *tile.Inventory)
    {
        if (Grid::GetNeededItemsToBuild(tile.Type, item.first) - item.second <= 0) continue;

        if (unit.Inventory->at(item.first) > 0) return true;
    }

    return false;
}

void UnitManager::SetGrid(Grid *grid)
{
    if (_grid == nullptr)
    {
        _units = std::vector<Unit>();
    }

	_grid = grid;
}

void UnitManager::LogTotalItems()
{
    for (int i = 0; i < 2; i++)
    {
        LOG("                                                           ");
    }

    LOG("Total items:                                                    ");
    for (auto pair: GetAllUsableItems())
    {
        LOG(Texture::ItemToString[(int) pair.first] << ": " << std::to_string(pair.second) + "                                  ");
    }

    for (int i = 0; i < 10; i++)
    {
        LOG("                                                           ");
    }
}
