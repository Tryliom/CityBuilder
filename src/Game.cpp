#include <cmath>
#include <iostream>

#include "sokol_app.h"

#include "Window.h"
#include "Audio.h"
#include "Input.h"
#include "Timer.h"
#include "Unit.h"
#include "Random.h"

void GenerateMap();

void UpdateCamera();
void HandleInput();
void DrawUi();

void UpdateUnits();
void DrawUnits();

void OnTickUnitSawMill(Unit& unit);
void OnTickUnitBuilderHut(Unit& unit);

Characters GetCharacter(int jobTileIndex);
bool IsTileTakenCareBy(TilePosition position, Characters character);
bool IsTileJobFull(int jobTileIndex);
int CountHowManyUnitAreWorkingOn(int jobTileIndex);

float speed = 500.f;

float unitSpeed = 100.f;
int unitSize = 10;
float unitProgress = 0.f;

Grid grid(5000, 5000, 100);
std::vector<Unit> units = std::vector<Unit>();

TileType selectedTileType = TileType::Sawmill;

void InitGame()
{
    std::cout << "Start Function" << std::endl;

    /*Audio::SetupSound();

    SoundClip testTheme = Audio::loadSoundClip(ASSETS_PATH "testTheme.wav");*/

    // Audio::PlaySoundClip(testTheme, 1.f, 440, 0, 0, true);

    Vector2F myVec(3, 4);
    Matrix2x3F id = Matrix2x3F::IdentityMatrix();
    Matrix2x3F tr = Matrix2x3F::TranslationMatrix({15, 10});
    Matrix2x3F rot = Matrix2x3F::RotationMatrix(90);
    Matrix2x3F sc = Matrix2x3F::ScaleMatrix({2, 2});

    std::cout << Matrix2x3F::Multiply(id, myVec).X << " " << Matrix2x3F::Multiply(id, myVec).Y << std::endl;
    std::cout << Matrix2x3F::Multiply(tr, myVec).X << " " << Matrix2x3F::Multiply(tr, myVec).Y << std::endl;
    std::cout << Matrix2x3F::Multiply(rot, myVec).X << " " << Matrix2x3F::Multiply(rot, myVec).Y << std::endl;
    std::cout << Matrix2x3F::Multiply(sc, myVec).X << " " << Matrix2x3F::Multiply(sc, myVec).Y << std::endl;

    Matrix2x3F mutlipled = Matrix2x3F::Multiply(rot, tr);
    std::cout << Matrix2x3F::Multiply(mutlipled, myVec).X << " " << Matrix2x3F::Multiply(mutlipled, myVec).Y << std::endl;

    Matrix2x3F test =
        {
            .values =
                {
                    14.f, 42.f, 73.f,
                    44.f, 5.f, -16.f}};

    Matrix2x3F oui = Matrix2x3F::Invert(test);
    Matrix2x3F transformatrix = Matrix2x3F::TransformMatrix({2, 2}, 90, {15, 10});

    std::cout << "\n\n" << Matrix2x3F::Multiply(transformatrix, myVec).X << " " << Matrix2x3F::Multiply(transformatrix, myVec).Y << std::endl;

    transformatrix = Matrix2x3F::TransformMatrix({6, -3}, -90, {39, 45});
    std::cout << "\n\n" << Matrix2x3F::Multiply(transformatrix, myVec).X << " " << Matrix2x3F::Multiply(transformatrix, myVec).Y << std::endl;

	GenerateMap();
}

void OnFrame()
{
    auto mousePosition = Input::GetMousePosition();

    UpdateCamera();
	HandleInput();

    Window::CalculTransformationMatrix();

    grid.Update();
    grid.Draw();

    UpdateUnits();
    DrawUnits();

	DrawUi();
}

void UpdateUnits()
{
    for (auto& unit : units)
    {
        if (unit.JobTileIndex != -1)
        {
            Tile& tile = grid.GetTile(unit.JobTileIndex);

            unit.TimeSinceLastAction += Timer::SmoothDeltaTime;

			// Always the same for all units
	        if (unit.CurrentBehavior == UnitBehavior::Moving)
	        {
		        auto targetPosition = grid.ToWorldPosition(unit.TargetTile);

				//TODO: Implement A* pathfinding

		        // Make it move to his target position
		        unit.Position += (targetPosition - unit.Position).Normalized() * unitSpeed * Timer::SmoothDeltaTime;

		        // Check if it reached his target position
		        if (std::abs(unit.Position.X - targetPosition.X) < 1.f && std::abs(unit.Position.Y - targetPosition.Y) < 1.f)
		        {
			        unit.SetBehavior(UnitBehavior::Working);
		        }
	        }

            if (tile.Type == TileType::Sawmill) OnTickUnitSawMill(unit);
			if (tile.Type == TileType::BuilderHut) OnTickUnitBuilderHut(unit);
        }
		else
        {
			auto tryToGetJobFor = [&](TileType type)
			{
				auto jobTiles = grid.GetTiles(type);

				for (auto& jobTile : jobTiles)
				{
					int jobTileIndex = grid.GetTileIndex(jobTile);

					if (IsTileJobFull(jobTileIndex)) continue;

					unit.JobTileIndex = jobTileIndex;
				}
			};

			auto jobs =
			{
				TileType::Sawmill,
				TileType::BuilderHut,
				TileType::Quarry,
			};

			for (auto& job : jobs)
			{
				tryToGetJobFor(job);

				if (unit.JobTileIndex != -1) break;
			}
		}
    }

	// Check if there is enough place for a new unit
	int housesCount = grid.GetTiles(TileType::House).size();

	if (units.size() < housesCount * 3)
	{
		unitProgress += Timer::SmoothDeltaTime;

		if (unitProgress > 20.f)
		{
			unitProgress = 0.f;

			units.push_back(Unit{.Position = grid.ToWorldPosition(grid.GetTiles(TileType::MayorHouse)[0])});
		}
	}
}

void OnTickUnitSawMill(Unit& unit)
{
	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		// Check if there is a full tree
		auto treePositions = grid.GetTiles(TileType::Tree, grid.GetTilePosition(unit.JobTileIndex), 4);

		for (auto& treePosition : treePositions)
		{
			auto& treeTile = grid.GetTile(treePosition);

			if (treeTile.TreeGrowth < 30.f || IsTileTakenCareBy(treePosition, Characters::Lumberjack)) continue;

			unit.TargetTile = treePosition;
			unit.SetBehavior(UnitBehavior::Moving);
		}
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		if (unit.TimeSinceLastAction > 2.f)
		{
			Tile& tree = grid.GetTile(unit.TargetTile);

			tree.TreeGrowth = 0.f;

			unit.SetBehavior(UnitBehavior::Idle);
		}
	}
}

void OnTickUnitBuilderHut(Unit& unit)
{
	if (unit.CurrentBehavior == UnitBehavior::Idle)
	{
		// Check if there is a construction to build
		TilePosition constructionPosition {-1, -1};

		grid.ForEachTile([&](Tile& tile, TilePosition position)
		{
			if (tile.Type != TileType::None && !tile.IsBuilt && !IsTileTakenCareBy(position, Characters::Builder))
			{
				constructionPosition = position;
				return true;
			}

			return false;
		});

		// Check if there is something to destroy
		if (constructionPosition.X == -1)
		{
			grid.ForEachTile([&](Tile& tile, TilePosition position)
			{
				if (tile.Type != TileType::None && tile.IsBuilt && tile.NeedToBeDestroyed && !IsTileTakenCareBy(position, Characters::Builder))
				{
					constructionPosition = position;
					return true;
				}

				return false;
			});
		}

		if (constructionPosition.X == -1) return;

		unit.TargetTile = constructionPosition;
		unit.SetBehavior(UnitBehavior::Moving);
	}
	else if (unit.CurrentBehavior == UnitBehavior::Working)
	{
		Tile& construction = grid.GetTile(unit.TargetTile);

		if (construction.IsBuilt && !construction.NeedToBeDestroyed || construction.Type == TileType::None)
		{
			unit.SetBehavior(UnitBehavior::Idle);
			return;
		}

		construction.Progress += Timer::SmoothDeltaTime;
	}
}

void DrawUnits()
{
    for (auto& unit : units)
    {
        Characters character = GetCharacter(unit.JobTileIndex);

        Window::DrawObject({
            .Position = unit.Position,
            .Size = {unitSize, unitSize},
            .Texture = Texture(character),
        });
    }
}

void UpdateCamera()
{
    auto mousePosition = Input::GetMousePosition();
    auto previousMousePosition = Input::GetPreviousMousePosition();
    auto smoothDeltaTime = Timer::SmoothDeltaTime;
    auto movementValue = speed * smoothDeltaTime;
    auto mouseWorldPosition = Window::ScreenToWorld(mousePosition);

    if (Input::IsKeyHeld(SAPP_KEYCODE_A))
    {
        Window::MoveCamera({movementValue, 0});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_D))
    {
        Window::MoveCamera({-movementValue, 0});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_W))
    {
        Window::MoveCamera({0, movementValue});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_S))
    {
        Window::MoveCamera({0, -movementValue});
    }

    if (Input::IsKeyPressed(SAPP_KEYCODE_Z))
    {
        Window::Zoom(0.1f);
    }
    if (Input::IsKeyPressed(SAPP_KEYCODE_X))
    {
        Window::Zoom(-0.1f);
    }

    Window::Zoom(Input::GetMouseWheelDelta() / 50.f);

    if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_MIDDLE))
	{
		Window::MoveCamera((mousePosition - previousMousePosition) * 1.f / Window::GetZoom());
	}
}

void HandleInput()
{
	if (Input::IsKeyPressed(SAPP_KEYCODE_1))
	{
		selectedTileType = TileType::Sawmill;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_2))
	{
		selectedTileType = TileType::BuilderHut;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_3))
	{
		selectedTileType = TileType::Quarry;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_4))
	{
		selectedTileType = TileType::Storage;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_5))
	{
		selectedTileType = TileType::House;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_6))
	{
		selectedTileType = TileType::Road;
	}

	if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_LEFT))
	{
		auto mousePosition = Input::GetMousePosition();
		auto mouseWorldPosition = Window::ScreenToWorld(mousePosition);
		auto tilePosition = grid.GetTilePosition(mouseWorldPosition);

		if (grid.CanBuild(tilePosition, selectedTileType))
		{
			grid.SetTile(tilePosition, Tile(selectedTileType));
		}
	}

	if (Input::IsMouseButtonPressed(SAPP_MOUSEBUTTON_RIGHT))
	{
		// Remove some tiles with permissions and other are set to be destroyed by a builder
		auto mousePosition = Input::GetMousePosition();
		auto mouseWorldPosition = Window::ScreenToWorld(mousePosition);
		auto tilePosition = grid.GetTilePosition(mouseWorldPosition);
		auto& tile = grid.GetTile(tilePosition);

		if (tile.Type == TileType::None) return;

		// Can be destroyed immediately
		if (tile.Type == TileType::Road || !tile.IsBuilt)
		{
			tile.Type = tile.Type == TileType::Quarry ? TileType::Stone : TileType::None;
			tile.IsBuilt = true;
			tile.NeedToBeDestroyed = false;
			tile.Progress = 0;
			return;
		}

		// Can be destroyed by a builder
		if (tile.Type == TileType::Stone || tile.Type == TileType::Tree || tile.Type == TileType::Storage || tile.Type == TileType::House || tile.Type == TileType::BuilderHut || tile.Type == TileType::Sawmill || tile.Type == TileType::Quarry)
		{
			tile.Progress = 0;
			tile.NeedToBeDestroyed = true;
		}
	}
}

void DrawUi()
{
	Texture selectedTileTexture;

	switch (selectedTileType)
	{
		case TileType::Sawmill:
			selectedTileTexture = Texture(Buildings::Sawmill);
			break;
		case TileType::BuilderHut:
			selectedTileTexture = Texture(Buildings::BuilderHut);
			break;
		case TileType::Quarry:
			selectedTileTexture = Texture(Buildings::Quarry);
			break;
		case TileType::Storage:
			selectedTileTexture = Texture(Buildings::Storage);
			break;
		case TileType::House:
			selectedTileTexture = Texture(Buildings::House);
			break;
		case TileType::Road:
			selectedTileTexture = Texture(Road::Single);
			break;
	}

	// Draw the select tile type at the top left
	Window::DrawRect(Window::ScreenToWorld({5, 5}), {110, 110}, {0.2f, 0.2f, 0.2f, 0.5f});
	Window::DrawObject({
		.Position = Window::ScreenToWorld({10, 10}),
		.Size = {100, 100},
		.Texture = selectedTileTexture,
	});
}

void GenerateMap()
{
	Vector2F centerOfScreen = Vector2F{ sapp_widthf(), sapp_heightf() } / 2.f;

	auto mayorHouse = Tile(TileType::MayorHouse);
	mayorHouse.IsBuilt = true;
	grid.SetTile(grid.GetTilePosition(centerOfScreen), mayorHouse);

	auto builderHouse = Tile(TileType::BuilderHut);
	builderHouse.IsBuilt = true;
	grid.SetTile(grid.GetTilePosition(centerOfScreen) + TilePosition{ 2, 0}, builderHouse);

	// Generate a random seed for the map
	Random::SetSeed(Random::Range(0, 1000000));
	Random::UseSeed();

	// Place random trees
	grid.ForEachTile([&](Tile& tile, TilePosition position)
	{
		if (tile.Type == TileType::None)
		{
			if (Random::Range(0, 100) < 10)
			{
				tile.Type = TileType::Tree;
				tile.TreeGrowth = Random::Range(0.f, 30.f);
				tile.IsBuilt = true;
			}
		}
	});

	// Place random rocks
	grid.ForEachTile([&](Tile& tile, TilePosition position)
	{
		if (tile.Type == TileType::None)
		{
			if (Random::Range(0, 100) < 10)
			{
				tile.Type = TileType::Stone;
				tile.IsBuilt = true;
			}
		}
	});

	Random::StopUseSeed();

	for (int i = 0; i < 2; i++)
	{
		units.push_back(Unit{.Position = grid.ToWorldPosition(grid.GetTiles(TileType::MayorHouse)[0])});
	}
}

Characters GetCharacter(int jobTileIndex)
{
	if (jobTileIndex == -1) return Characters::Unemployed;

	Tile& jobTile = grid.GetTile(jobTileIndex);

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

	return Characters::Unemployed;
}

bool IsTileTakenCareBy(TilePosition position, Characters character)
{
	for (auto& unit : units)
	{
		if (unit.JobTileIndex == -1) continue;

		if (unit.CurrentBehavior == UnitBehavior::Moving || unit.CurrentBehavior == UnitBehavior::Working && unit.TargetTile == position && GetCharacter(unit.JobTileIndex) == character)
		{
			return true;
		}
	}

	return false;
}

bool IsTileJobFull(int jobTileIndex)
{
	Tile& tile = grid.GetTile(jobTileIndex);

	if (!tile.IsBuilt) return true;

	switch (tile.Type)
	{
		case TileType::Sawmill: return CountHowManyUnitAreWorkingOn(jobTileIndex) >= 3;
		case TileType::BuilderHut: return CountHowManyUnitAreWorkingOn(jobTileIndex) >= 1;
		case TileType::Quarry: return CountHowManyUnitAreWorkingOn(jobTileIndex) >= 2;

		default: return true;
	}
}

int CountHowManyUnitAreWorkingOn(int jobTileIndex)
{
	int result = 0;

	for (auto& unit : units)
	{
		if (unit.JobTileIndex == jobTileIndex) result++;
	}

	return result;
}