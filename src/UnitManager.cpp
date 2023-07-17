#include "UnitManager.h"
#include "Graphics.h"
#include "Timer.h"

float unitSpeed = 100.f;
int unitSize = 10;
float unitProgress;

UnitManager::UnitManager(Grid& grid) : _grid(grid) {}

void UnitManager::AddUnit(Unit unit)
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
					//TODO: Implement A* pathfinding when choosing the target to get the shortest path

					// Make it move to his target position
					unit.Position += (targetPosition - unit.Position).Normalized() * unitSpeed * Timer::SmoothDeltaTime;

					// Check if it reached his target position
					if (std::abs(unit.Position.X - targetPosition.X) < 1.f && std::abs(unit.Position.Y - targetPosition.Y) < 1.f)
					{
						unit.SetBehavior(UnitBehavior::Working);
					}
				}
			}

			if (tile.Type == TileType::Sawmill) OnTickUnitSawMill(unit);
			if (tile.Type == TileType::BuilderHut) OnTickUnitBuilderHut(unit);
			if (tile.Type == TileType::LogisticsCenter) onTickUnitLogistician(unit);
			//TODO: Add miner
		}
		else
		{
			auto tryToGetJobFor = [&](TileType type)
			{
				auto jobTiles = _grid.GetTiles(type);

				for (auto& jobTile : jobTiles)
				{
					int jobTileIndex = _grid.GetTileIndex(jobTile);

					if (IsTileJobFull(jobTileIndex)) continue;

					unit.JobTileIndex = jobTileIndex;
				}
			};

			auto jobs =
				{
					TileType::Sawmill,
					TileType::BuilderHut,
					TileType::Quarry,
					TileType::LogisticsCenter
				};

			for (auto& job : jobs)
			{
				tryToGetJobFor(job);

				if (unit.JobTileIndex != -1) break;
			}
		}

		if (unit.Logs > GetMaxLogsFor(unit))
		{
			unit.Logs = GetMaxLogsFor(unit);
		}

		if (unit.Rocks > GetMaxRocksFor(unit))
		{
			unit.Rocks = GetMaxRocksFor(unit);
		}
	}

	// Check if there is enough place for a new unit
	int housesCount = _grid.GetTiles(TileType::House).size();

	if (_units.size() < housesCount * 3)
	{
		unitProgress += Timer::SmoothDeltaTime;

		if (unitProgress > 20.f)
		{
			unitProgress = 0.f;

			_units.push_back(Unit{.Position = _grid.ToWorldPosition(_grid.GetTiles(TileType::MayorHouse)[0])});
		}
	}
}

void UnitManager::OnTickUnitSawMill(Unit& unit)
{
	Tile& jobTile = _grid.GetTile(unit.JobTileIndex);

	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		if (unit.Logs >= GetMaxLogsFor(unit) / 2 && jobTile.Logs < Grid::GetMaxLogsStored(jobTile))
		{
			// Go back to the sawmill to drop the logs
			unit.TargetTile = _grid.GetTilePosition(unit.JobTileIndex);
			unit.SetBehavior(UnitBehavior::Moving);
		}

		// Check if there is a full tree
		auto treePositions = _grid.GetTiles(TileType::Tree, _grid.GetTilePosition(unit.JobTileIndex), 3);

		for (auto& treePosition : treePositions)
		{
			auto& treeTile = _grid.GetTile(treePosition);

			if (treeTile.TreeGrowth < 30.f || IsTileTakenCareBy(treePosition, Characters::Lumberjack)) continue;

			unit.TargetTile = treePosition;
			unit.SetBehavior(UnitBehavior::Moving);
		}

		if (unit.CurrentBehavior != UnitBehavior::Moving)
		{
			if (unit.Logs > 0 && jobTile.Logs < Grid::GetMaxLogsStored(jobTile))
			{
				// Go back to the sawmill to drop the logs
				unit.TargetTile = _grid.GetTilePosition(unit.JobTileIndex);
				unit.SetBehavior(UnitBehavior::Moving);
			}
		}
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		Tile& tile = _grid.GetTile(unit.TargetTile);

		if (tile.Type == TileType::Sawmill)
		{
			// Drop the logs in the sawmill
			int spaceLeft = Grid::GetMaxLogsStored(jobTile) - jobTile.Logs;
			int logsToDrop = std::min(unit.Logs, spaceLeft);

			unit.Logs -= logsToDrop;
			tile.Logs += logsToDrop;

			unit.SetBehavior(UnitBehavior::Idle);
		}
		else if (unit.TimeSinceLastAction > 2.f)
		{
			tile.TreeGrowth = 0.f;

			unit.Logs += 5;

			unit.SetBehavior(UnitBehavior::Idle);
		}
	}
}

void UnitManager::OnTickUnitBuilderHut(Unit& unit)
{
	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		// Check if he has something in his inventory
		if (unit.Logs > 0 || unit.Rocks > 0)
		{
			// Search for a storage free space to drop the resources, first search to drop the logs, then the rocks
			auto storagePositions = _grid.GetTiles(TileType::Storage, _grid.GetTilePosition(unit.JobTileIndex), 5);
			Tile& mayorHouseTile = _grid.GetTile(_grid.GetTiles(TileType::MayorHouse)[0]);

			if (unit.Logs > 0)
			{
				if (mayorHouseTile.Logs < Grid::GetMaxLogsStored(mayorHouseTile))
				{
					unit.TargetTile = _grid.GetTiles(TileType::MayorHouse)[0];
					unit.SetBehavior(UnitBehavior::Moving);
				}
				else
				{
					for (auto& storagePosition: storagePositions)
					{
						auto& storageTile = _grid.GetTile(storagePosition);

						if (storageTile.Logs < Grid::GetMaxLogsStored(storageTile))
						{
							unit.TargetTile = storagePosition;
							unit.SetBehavior(UnitBehavior::Moving);
							break;
						}
					}
				}
			}

			if (unit.CurrentBehavior == UnitBehavior::Idle && unit.Rocks > 0)
			{
				if (mayorHouseTile.Rocks < Grid::GetMaxRocksStored(mayorHouseTile))
				{
					unit.TargetTile = _grid.GetTiles(TileType::MayorHouse)[0];
					unit.SetBehavior(UnitBehavior::Moving);
				}
				else
				{
					for (auto& storagePosition: storagePositions)
					{
						auto& storageTile = _grid.GetTile(storagePosition);

						if (storageTile.Rocks < Grid::GetMaxRocksStored(storageTile))
						{
							unit.TargetTile = storagePosition;
							unit.SetBehavior(UnitBehavior::Moving);
							break;
						}
					}
				}
			}
		}

		if (unit.CurrentBehavior != UnitBehavior::Idle) return;

		// Check if there is a construction to build
		TilePosition constructionPosition {-1, -1};
		bool hasFound = false;

		_grid.ForEachTile([&](Tile& tile, TilePosition position)
	     {
	         if (tile.Type != TileType::None && !IsTileTakenCareBy(position, Characters::Builder)
	             && (!tile.IsBuilt && GetNeededLogsFor(tile.Type) == tile.Logs && GetNeededRocksFor(tile.Type) == tile.Rocks)
	             || (tile.NeedToBeDestroyed && tile.IsBuilt))
	         {
	             constructionPosition = position;
	             hasFound = true;
	             return true;
	         }

	         return false;
	     });

		// Check if there is something to build/destroy
		if (hasFound)
		{
			unit.TargetTile = constructionPosition;
			unit.SetBehavior(UnitBehavior::Moving);
		}
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		Tile& tile = _grid.GetTile(unit.TargetTile);

		if (!tile.NeedToBeDestroyed && tile.IsBuilt)
		{
			if (tile.Type == TileType::MayorHouse || tile.Type == TileType::Storage || tile.Type == TileType::LogisticsCenter)
			{
				// Drop the logs and rocks in the storage
				int spaceLeft = GetMaxLogsFor(unit) - tile.Logs;
				int logsToDrop = std::min(unit.Logs, spaceLeft);

				unit.Logs -= logsToDrop;
				tile.Logs += logsToDrop;

				spaceLeft = GetMaxRocksFor(unit) - tile.Rocks;
				int rocksToDrop = std::min(unit.Rocks, spaceLeft);

				unit.Rocks -= rocksToDrop;
				tile.Rocks += rocksToDrop;
			}

			unit.SetBehavior(UnitBehavior::Idle);
			return;
		}

		tile.Progress += Timer::SmoothDeltaTime;

		if (!tile.IsBuilt && tile.Progress >= Grid::GetMaxConstructionProgress(tile.Type))
		{
			tile.IsBuilt = true;
			tile.NeedToBeDestroyed = false;
			tile.Progress = 0.f;
			tile.Logs = 0;
			tile.Rocks = 0;

			unit.SetBehavior(UnitBehavior::Idle);
		}
		else if (tile.NeedToBeDestroyed && tile.Progress >= Grid::GetMaxDestructionProgress(tile.Type))
		{
			tile.IsBuilt = false;
			tile.NeedToBeDestroyed = false;
			tile.Progress = 0.f;
			tile.Logs = 0;
			tile.Rocks = 0;

			unit.SetBehavior(UnitBehavior::Idle);

			if (tile.Type == TileType::Tree)
			{
				unit.Logs += 5;
			}
			else if (tile.Type == TileType::Stone)
			{
				unit.Rocks += 20;
			}

			tile.Type = TileType::None;
		}
	}
}

void UnitManager::onTickUnitLogistician(Unit& unit)
{
	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		// Check if there is a construction to build that need resources
		TilePosition constructionPosition {};
		bool hasFoundConstruction = false;

		_grid.ForEachTile([&](Tile& tile, TilePosition position)
         {
             if (tile.Type != TileType::None && !tile.IsBuilt && !IsTileTakenCareBy(position, Characters::Logistician)
                 && (GetNeededLogsFor(tile.Type) > tile.Logs || GetNeededRocksFor(tile.Type) > tile.Rocks))
             {
                 constructionPosition = position;
                 hasFoundConstruction = true;
                 return true;
             }

             return false;
         });

		Tile& construction = _grid.GetTile(constructionPosition);

		// If there is a construction to build that need resources, check if the unit has the resources to build it or is full
		if (hasFoundConstruction)
		{
			int logsNeeded = GetNeededLogsFor(construction.Type) - construction.Logs;
			int rocksNeeded = GetNeededRocksFor(construction.Type) - construction.Rocks;

			if ((logsNeeded > 0 && unit.Logs > 0)
			    || (rocksNeeded > 0 && unit.Rocks > 0))
			{
				unit.TargetTile = constructionPosition;
				unit.SetBehavior(UnitBehavior::Moving);
			}
				// Then check where it can pull resources to build it
			else
			{
				_grid.ForEachTile([&](Tile& tile, TilePosition position)
                 {
	                 if ((tile.Type == TileType::Storage || tile.Type == TileType::MayorHouse || tile.Type == TileType::LogisticsCenter) && logsNeeded > 0 ? tile.Logs > 0 : tile.Rocks > 0)
	                 {
		                 unit.TargetTile = position;
		                 unit.SetBehavior(UnitBehavior::Moving);
		                 return true;
	                 }

	                 return false;
                 });
			}
		}

		if (unit.CurrentBehavior != UnitBehavior::Idle) return;

		// Check if the unit has resources on him and if there is space in storages to drop them
		if (unit.Logs > 0 || unit.Rocks > 0)
		{
			_grid.ForEachTile([&](Tile& tile, TilePosition position)
             {
                 if ((tile.Type == TileType::Storage || tile.Type == TileType::LogisticsCenter || tile.Type == TileType::MayorHouse)
				    && unit.Logs > 0 ? tile.Logs < Grid::GetMaxLogsStored(tile) : tile.Rocks < Grid::GetMaxRocksStored(tile))
                 {
	                 unit.TargetTile = position;
	                 unit.SetBehavior(UnitBehavior::Moving);

	                 return true;
                 }

                 return false;
             });
		}

		if (unit.CurrentBehavior != UnitBehavior::Idle) return;

		// Check if there is logs in the sawmill or rocks in the quarry to move to the storage
		auto sawmillPositions = _grid.GetTiles(TileType::Sawmill);

		for (auto& sawmillPosition : sawmillPositions)
		{
			auto& sawmillTile = _grid.GetTile(sawmillPosition);

			if (sawmillTile.Logs > 0)
			{
				unit.TargetTile = sawmillPosition;
				unit.SetBehavior(UnitBehavior::Moving);
				break;
			}
		}

		if (unit.CurrentBehavior != UnitBehavior::Idle) return;

		auto quarryPositions = _grid.GetTiles(TileType::Quarry);

		for (auto& quarryPosition : quarryPositions)
		{
			auto& quarryTile = _grid.GetTile(quarryPosition);

			if (quarryTile.Rocks > 0)
			{
				unit.TargetTile = quarryPosition;
				unit.SetBehavior(UnitBehavior::Moving);
				break;
			}
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
			int logsNeeded = GetNeededLogsFor(tile.Type) - tile.Logs;
			int rocksNeeded = GetNeededRocksFor(tile.Type) - tile.Rocks;

			if (logsNeeded > 0)
			{
				int logsToDrop = std::min(unit.Logs, logsNeeded);
				unit.Logs -= logsToDrop;
				tile.Logs += logsToDrop;
			}

			if (rocksNeeded > 0)
			{
				int rocksToDrop = std::min(unit.Rocks, rocksNeeded);
				unit.Rocks -= rocksToDrop;
				tile.Rocks += rocksToDrop;
			}
		}

			// If it's a storage, first check if there is something to build that need resources
		else if (tile.Type == TileType::Storage || tile.Type == TileType::MayorHouse || tile.Type == TileType::LogisticsCenter)
		{
			TilePosition constructionPosition {};
			bool hasFoundConstruction = false;

			_grid.ForEachTile([&](Tile& tile, TilePosition position)
	         {
	             if (tile.Type != TileType::None && !tile.IsBuilt && !IsTileTakenCareBy(position, Characters::Logistician)
	                 && (GetNeededLogsFor(tile.Type) > tile.Logs || GetNeededRocksFor(tile.Type) > tile.Rocks))
	             {
	                 constructionPosition = position;
	                 hasFoundConstruction = true;
	                 return true;
	             }

	             return false;
	         });

			Tile& construction = _grid.GetTile(constructionPosition);

			// If there is a construction to build that need resources, check if the unit has the resources to build it or is full
			if (hasFoundConstruction)
			{
				if ((unit.Logs != GetMaxLogsFor(unit) && unit.Logs < GetNeededLogsFor(construction.Type) - construction.Logs)
				    || (unit.Rocks != GetMaxRocksFor(unit) && unit.Rocks < GetNeededRocksFor(construction.Type) - construction.Rocks))
				{
					int logsNeeded = GetNeededLogsFor(construction.Type) - construction.Logs;
					int rocksNeeded = GetNeededRocksFor(construction.Type) - construction.Rocks;

					// Move the needed resources from the storage to the unit
					if (unit.Logs != GetMaxLogsFor(unit))
					{
						int logsToGet = std::min(logsNeeded, GetMaxLogsFor(unit) - unit.Logs);
						logsToGet = std::min(logsToGet, tile.Logs);

						unit.Logs += logsToGet;
						tile.Logs -= logsToGet;
					}

					if (unit.Rocks != GetMaxRocksFor(unit))
					{
						int rocksToGet = std::min(rocksNeeded, GetMaxRocksFor(unit) - unit.Rocks);
						rocksToGet = std::min(rocksToGet, tile.Rocks);

						unit.Rocks += rocksToGet;
						tile.Rocks -= rocksToGet;
					}
				}
			}
				// Check if there is resources to move to the storage
			else
			{
				int logsToDrop = std::min(unit.Logs, Grid::GetMaxLogsStored(tile) - tile.Logs);

				unit.Logs -= logsToDrop;
				tile.Logs += logsToDrop;

				int rocksToDrop = std::min(unit.Rocks, Grid::GetMaxRocksStored(tile) - tile.Rocks);

				unit.Rocks -= rocksToDrop;
				tile.Rocks += rocksToDrop;
			}
		}
			// If it's a sawmill or a quarry, check if there is resources to get from it
		else if (tile.Type == TileType::Sawmill || tile.Type == TileType::Quarry)
		{
			int logsToGet = std::min(GetMaxLogsFor(unit) - unit.Logs, tile.Logs);
			int rocksToGet = std::min(GetMaxRocksFor(unit) - unit.Rocks, tile.Rocks);

			unit.Logs += logsToGet;
			unit.Rocks += rocksToGet;

			tile.Logs -= logsToGet;
			tile.Rocks -= rocksToGet;
		}

		unit.SetBehavior(UnitBehavior::Idle);
	}
}

void UnitManager::DrawUnits()
{
	for (auto& unit : _units)
	{
		Characters character = GetCharacter(unit.JobTileIndex);

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

		if (unit.CurrentBehavior == UnitBehavior::Moving || unit.CurrentBehavior == UnitBehavior::Working && unit.TargetTile == position && GetCharacter(unit.JobTileIndex) == character)
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

	switch (tile.Type)
	{
		case TileType::Sawmill: return CountHowManyUnitAreWorkingOn(jobTileIndex) >= 3;
		case TileType::BuilderHut: return CountHowManyUnitAreWorkingOn(jobTileIndex) >= 1;
		case TileType::Quarry: return CountHowManyUnitAreWorkingOn(jobTileIndex) >= 2;
		case TileType::LogisticsCenter: return CountHowManyUnitAreWorkingOn(jobTileIndex) >= 1;

		default: return true;
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

int UnitManager::GetMaxLogsFor(Unit& unit)
{
	if (unit.JobTileIndex == -1) return 0;

	Tile& tile = _grid.GetTile(unit.JobTileIndex);

	if (!tile.IsBuilt) return 0;

	switch (tile.Type)
	{
		case TileType::Sawmill: return 30;
		case TileType::BuilderHut: return 15;
		case TileType::Quarry: return 0;
		case TileType::LogisticsCenter: return 50;

		default: return 0;
	}
}

int UnitManager::GetMaxRocksFor(Unit& unit)
{
	if (unit.JobTileIndex == -1) return 0;

	Tile& tile = _grid.GetTile(unit.JobTileIndex);

	if (!tile.IsBuilt) return 0;

	switch (tile.Type)
	{
		case TileType::Sawmill: return 0;
		case TileType::BuilderHut: return 15;
		case TileType::Quarry: return 30;
		case TileType::LogisticsCenter: return 50;

		default: return 0;
	}
}

int UnitManager::GetNeededLogsFor(TileType tileType)
{
	switch (tileType)
	{
		case TileType::Sawmill: return 10;
		case TileType::BuilderHut: return 20;
		case TileType::Quarry: return 20;
		case TileType::LogisticsCenter: return 20;
		case TileType::House: return 30;
		case TileType::Storage: return 20;

		default: return 0;
	}
}

int UnitManager::GetNeededRocksFor(TileType tileType)
{
	switch (tileType)
	{
		case TileType::Sawmill: return 0;
		case TileType::BuilderHut: return 10;
		case TileType::Quarry: return 0;
		case TileType::LogisticsCenter: return 10;
		case TileType::House: return 15;

		default: return 0;
	}
}