#pragma once

#include <vector>

#include "Unit.h"

class UnitManager
{
public:
	UnitManager() = default;

private:
	Grid* _grid;
	std::vector<Unit> _units;

	void OnTickUnitSawMill(Unit& unit);
	void OnTickUnitBuilderHut(Unit& unit);
	void onTickUnitLogistician(Unit& unit);

	Characters GetCharacter(int jobTileIndex);
	bool IsTileTakenCareBy(TilePosition position, Characters character);
	bool IsTileJobFull(int jobTileIndex);
	int CountHowManyUnitAreWorkingOn(int jobTileIndex);
	int GetMaxLogsFor(Unit& unit);
	int GetMaxRocksFor(Unit& unit);

	static int GetNeededLogsFor(TileType tileType);
	static int GetNeededRocksFor(TileType tileType);

public:
	void UpdateUnits();
	void DrawUnits();

	void AddUnit(Unit unit);

	void SetGrid(Grid* grid);
};