#include <algorithm>
#include "UnitManager.h"

#include "Window.h"
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

UnitManager::UnitManager(Grid& grid) : _grid(grid) {}

void UnitManager::AddUnit(const Unit& unit)
{
	_units.push_back(unit);
}

void UnitManager::UpdateUnits()
{
	for (auto& unit : _units)
	{
		if (unit.JobTileIndex != -1)
		{
			Tile& tile = _grid.GetTile(unit.JobTileIndex);

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
				auto targetPosition = _grid.ToWorldPosition(unit.TargetTile);

				if (targetPosition == unit.Position)
				{
					unit.SetBehavior(UnitBehavior::Working);
				}
				else
				{
					if (unit.PathToTargetTile.empty())
					{
						unit.PathToTargetTile = _grid.GetPath(_grid.GetTilePosition(unit.Position), unit.TargetTile);

						// Check if it's already on the target tile
						if (unit.PathToTargetTile.empty())
						{
							unit.SetBehavior(UnitBehavior::Working);
						}
					}
					else
					{
						TilePosition nextTilePosition = unit.PathToTargetTile.front();
						Vector2F nextTileWorldPosition = _grid.ToWorldPosition(nextTilePosition) + Vector2F(0.5f, 0.5f) * ((float) _grid.GetTileSize() - unitSize);
						float speedFactor = 1.f;

						// Check if the next tile is a road
						if (_grid.GetTile(nextTilePosition).Type == TileType::Road)
						{
							speedFactor = 1.5f;
						}

						speedFactor += Grid::GetSpeedFactor(tile.Type);

						// Check if it's the last tile to set the target position to the center-bottom of the tile
						if (unit.PathToTargetTile.size() == 1)
						{
							nextTileWorldPosition += Vector2F(0.f, 1.f) * ((float) _grid.GetTileSize()) / 2.f;
						}

						// Move it to the center of the next tile
						unit.Position += (nextTileWorldPosition - unit.Position).Normalized() * unitSpeed * speedFactor * Timer::SmoothDeltaTime;

						// Check if it reached the center of the next tile
						if (std::abs(unit.Position.GetDistance(nextTileWorldPosition)) < 3.f)
						{
							unit.PathToTargetTile.erase(unit.PathToTargetTile.begin());
						}

						// Check if it reached his target position
						if (unit.PathToTargetTile.empty())
						{
							unit.SetBehavior(UnitBehavior::Working);
						}
					}
				}
			}

			if (tile.Type == TileType::Sawmill) OnTickUnitSawMill(unit);
			if (tile.Type == TileType::BuilderHut) OnTickUnitBuilderHut(unit);
			if (tile.Type == TileType::LogisticsCenter) onTickUnitLogistician(unit);
			if (tile.Type == TileType::Quarry) OnTickUnitQuarry(unit);
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
	int housesCount = _grid.GetTiles(TileType::House).size();

	if (_units.size() < housesCount * 5)
	{
		unitProgress += Timer::SmoothDeltaTime;

		if (unitProgress > 20.f)
		{
			unitProgress = 0.f;

			_units.emplace_back(_grid.ToWorldPosition(_grid.GetTiles(TileType::MayorHouse)[0]));
		}
	}
}

void UnitManager::OnTickUnitSawMill(Unit& unit)
{
	Tile& jobTile = _grid.GetTile(unit.JobTileIndex);

	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		// Check if the unit need to drop items at the sawmill
		if (NeedToDropItemsAtJob(unit, Items::Wood, InventoryReason::MoreThanHalf))
		{
			unit.TargetTile = _grid.GetTilePosition(unit.JobTileIndex);
			unit.SetBehavior(UnitBehavior::Moving);
			return;
		}

		// Check if there is a full tree
		auto treePositions = GetAllHarvestableTrees(_grid.GetTilePosition(unit.JobTileIndex), 3);

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
			unit.TargetTile = _grid.GetTilePosition(unit.JobTileIndex);
			unit.SetBehavior(UnitBehavior::Moving);
			return;
		}
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		Tile& tile = _grid.GetTile(unit.TargetTile);

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
		Tile& tile = _grid.GetTile(unit.TargetTile);

		// Check that the tile is still valid
		if (!Grid::IsAStorage(tile.Type) && (tile.IsBuilt && !tile.NeedToBeDestroyed) || tile.Type == TileType::None)
		{
			unit.SetBehavior(UnitBehavior::Idle);
		}
	}

	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		// Check if he has something in his inventory
		if (!IsInventoryEmpty(unit))
		{
			// Search for a storage free space to drop the resources he has around his builder house
			for (auto pair : *unit.Inventory)
			{
				if (pair.second == 0) continue;

				auto storagePositions = GetStorageAroundFor(_grid.GetTilePosition(unit.JobTileIndex), 5, pair.first);

				unit.TargetTile = storagePositions[0];
				unit.SetBehavior(UnitBehavior::Moving);
				return;
			}
		}

		// Check if there is a construction to build
		auto workPositions = GetAllBuildableOrDestroyableTiles();

		if (!workPositions.empty())
		{
			unit.TargetTile = workPositions[0];
			unit.SetBehavior(UnitBehavior::Moving);
			return;
		}
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		Tile& tile = _grid.GetTile(unit.TargetTile);

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

			tile.Reset();
			unit.SetBehavior(UnitBehavior::Idle);
		}
	}
}

void UnitManager::onTickUnitLogistician(Unit& unit)
{
	if (unit.CurrentBehavior == UnitBehavior::Moving)
	{
		Tile& tile = _grid.GetTile(unit.TargetTile);

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
			for (auto pair : *_grid.GetTile(tilePosition).Inventory)
			{
				Items item = pair.first;
				int quantity = pair.second;

				int neededItems = Grid::GetNeededItemsToBuild(_grid.GetTile(tilePosition).Type, item);

				if (quantity >= neededItems) continue;

				int itemsToGet = neededItems - quantity;

				// Check if the unit has the resources in his inventory or has his inventory full of this item
				if (unit.Inventory->at(item) >= itemsToGet || unit.Inventory->at(item) == GetMaxItemsFor(unit, item)) continue;

				// Search for a storage that has the resources
				auto storagePositions = GetStorageThatHave(item);

				if (storagePositions.empty()) continue;

				unit.TargetTile = storagePositions[0];
				unit.SetBehavior(UnitBehavior::Moving);
				return;
			}

			if (!IsInventoryEmpty(unit))
			{
				unit.TargetTile = tilePosition;
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

				auto storagePositions = GetStorageAroundFor(_grid.GetTilePosition(unit.JobTileIndex), 20, pair.first);

				unit.TargetTile = storagePositions[0];
				unit.SetBehavior(UnitBehavior::Moving);
				return;
			}
		}

		std::vector<TilePosition> tilesToGetItemsFrom = {};
		auto sawmills = _grid.GetTilesWithItems(TileType::Sawmill, Items::Wood);
		auto quarries = _grid.GetTilesWithItems(TileType::Quarry, Items::Stone);

		tilesToGetItemsFrom.reserve(sawmills.size());

		for (auto tile : sawmills)
		{
			tilesToGetItemsFrom.push_back(tile);
		}

		for (auto tile : quarries)
		{
			tilesToGetItemsFrom.push_back(tile);
		}

		std::sort(tilesToGetItemsFrom.begin(), tilesToGetItemsFrom.end(), [&](TilePosition a, TilePosition b)
		{
			return _grid.GetTile(a).Inventory->at(Items::Wood) + _grid.GetTile(a).Inventory->at(Items::Stone) > _grid.GetTile(b).Inventory->at(Items::Wood) + _grid.GetTile(b).Inventory->at(Items::Stone);
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

		Tile& tile = _grid.GetTile(unit.TargetTile);

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

			if (!buildsThatNeedResources.empty())
			{
				auto tilePosition = buildsThatNeedResources[0];

				// Check if the unit has the resources to build it or need to go to a storage to get them
				for (auto pair : *_grid.GetTile(tilePosition).Inventory)
				{
					Items item = pair.first;
					int quantity = pair.second;

					int neededItems = Grid::GetNeededItemsToBuild(_grid.GetTile(tilePosition).Type, item);

					if (quantity >= neededItems) continue;

					int itemsToGet = neededItems - quantity;

					// Check if the unit has the resources in his inventory or has his inventory full of this item
					if (unit.Inventory->at(item) >= itemsToGet || unit.Inventory->at(item) == GetMaxItemsFor(unit, item)) continue;

					// Search for a storage that has the resources
					auto storagePositions = GetStorageThatHave(item);

					if (storagePositions.empty()) continue;

					// Take the items
					_grid.GetTile(storagePositions[0]).Inventory->at(item) -= itemsToGet;
					unit.Inventory->at(item) += itemsToGet;
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

		unit.SetBehavior(UnitBehavior::Idle);
	}
}

void UnitManager::OnTickUnitQuarry(Unit& unit)
{
	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		TilePosition tilePosition = _grid.GetTilePosition(unit.JobTileIndex);

		// Check if the quarry is full
		if (Grid::GetLeftSpaceForItems(_grid.GetTile(tilePosition), Items::Stone) == 0) return;

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

		Tile& tile = _grid.GetTile(unit.TargetTile);

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

void UnitManager::DrawUnits()
{
	for (auto& unit : _units)
	{
		Characters character = GetCharacter(unit.JobTileIndex);

		Window::DrawObject({
            .Position = unit.Position,
            .Size = {unitSize, unitSize},
            .Texture = Texture(character),
        });
	}
}

Characters UnitManager::GetCharacter(int jobTileIndex)
{
	if (jobTileIndex == -1) return Characters::Unemployed;

	Tile& jobTile = _grid.GetTile(jobTileIndex);

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
	Tile& tile = _grid.GetTile(jobTileIndex);

	if (!tile.IsBuilt) return true;

	return CountHowManyUnitAreWorkingOn(jobTileIndex) >= GetMaxUnitOnJob(jobTileIndex);
}

int UnitManager::GetMaxUnitOnJob(int jobTileIndex)
{
	Tile& tile = _grid.GetTile(jobTileIndex);

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


std::vector<TilePosition> UnitManager::GetAllHarvestableTrees(TilePosition position, int radius)
{
	std::vector<TilePosition> trees = std::vector<TilePosition>();
	auto treePositions = _grid.GetTiles(TileType::Tree, position, radius);

	for (auto& treePosition : treePositions)
	{
		auto& treeTile = _grid.GetTile(treePosition);

		if (treeTile.TreeGrowth < 30.f || IsTileTakenCareBy(treePosition, Characters::Lumberjack)) continue;

		trees.push_back(treePosition);
	}

	return trees;
}

bool UnitManager::NeedToDropItemsAtJob(Unit& unit, Items item, InventoryReason reason)
{
	Tile& jobTile = _grid.GetTile(unit.JobTileIndex);
	int maxItems = GetMaxItemsFor(unit, item);

	if (jobTile.Inventory->at(item) == Grid::GetMaxItemsStored(jobTile, item)) return false;
	if (reason == InventoryReason::Full && unit.Inventory->at(item) < maxItems) return false;
	if (reason == InventoryReason::MoreThanHalf && unit.Inventory->at(item) < maxItems / 2) return false;
	if (reason == InventoryReason::MoreThanOne && unit.Inventory->at(item) == 0) return false;

	return true;
}

std::vector<TilePosition> UnitManager::GetStorageAroundFor(TilePosition position, int radius, Items item)
{
	std::vector<TilePosition> storages = std::vector<TilePosition>();
	std::vector<TilePosition> storagePositions = _grid.GetTiles(position, radius);

	for (auto& storagePosition : storagePositions)
	{
		auto& storageTile = _grid.GetTile(storagePosition);

		if (!Grid::IsAStorage(storageTile.Type)) continue;
		if (storageTile.Inventory->at(item) == Grid::GetMaxItemsStored(storageTile, item)) continue;

		storages.push_back(storagePosition);
	}

	return storages;
}

std::vector<TilePosition> UnitManager::GetAllBuildableOrDestroyableTiles()
{
	std::vector<TilePosition> tiles = std::vector<TilePosition>();

	_grid.ForEachTile([&](Tile& tile, TilePosition position)
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

	_grid.ForEachTile([&](Tile& tile, TilePosition position)
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

std::vector<TilePosition> UnitManager::GetStorageThatHave(Items item)
{
	std::vector<TilePosition> storages = std::vector<TilePosition>();

	_grid.ForEachTile([&](Tile& tile, TilePosition position)
	{
		if (!Grid::IsAStorage(tile.Type)) return;
		if (tile.Inventory->at(item) == 0) return;

		storages.push_back(position);
	});

	return storages;
}

std::vector<int> UnitManager::GetAvailableJobs()
{
	std::vector<int> jobs = std::vector<int>();

	_grid.ForEachTile([&](Tile& tile, TilePosition position)
	{
		int index = _grid.GetTileIndex(position);

		if (tile.Type == TileType::None) return;
		if (IsTileJobFull(index)) return;

		jobs.push_back(index);
	});

	return jobs;
}

int UnitManager::GetMaxItemsFor(Unit& unit, Items item)
{
	if (unit.JobTileIndex == -1) return 0;

	Tile& tile = _grid.GetTile(unit.JobTileIndex);

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

std::map<Items, int> UnitManager::GetAllUsableItems()
{
	std::map<Items, int> items = std::map<Items, int>();

	for (int i = 0; i < (int)Items::Count; i++)
	{
		items.insert(std::pair<Items, int>((Items) i, 0));
	}

	// Add all items from logisticians
	for (auto& unit : _units)
	{
		if (unit.JobTileIndex == -1) continue;

		auto& tile = _grid.GetTile(unit.JobTileIndex);

		if (tile.Type != TileType::LogisticsCenter) continue;

		for (auto& item : *unit.Inventory)
		{
			items.at(item.first) += item.second;
		}
	}

	// Add all items from storages
	_grid.ForEachTile([&](Tile& tile, TilePosition position)
	{
		if (!Grid::IsAStorage(tile.Type)) return;

		for (auto& item : *tile.Inventory)
		{
			items.at(item.first) += item.second;
		}
	});

	return items;
}