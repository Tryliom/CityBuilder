#include <cmath>
#include <iostream>

#include "sokol_app.h"

#include "Window.h"
#include "Audio.h"
#include "Input.h"
#include "Timer.h"
#include "Unit.h"
#include "Random.h"
#include "Logger.h"
#include "UnitManager.h"

void GenerateMap();

void UpdateCamera();
void HandleInput();
void DrawUi();

float speed = 500.f;

Grid grid(5000, 5000, 100);
UnitManager unitManager(grid);

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

    unitManager.UpdateUnits();
	unitManager.DrawUnits();

	DrawUi();
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
	if (Input::IsKeyPressed(SAPP_KEYCODE_7))
	{
		selectedTileType = TileType::LogisticsCenter;
	}

	if (Input::IsKeyPressed(SAPP_KEYCODE_F))
	{
		// Spawn a unit
		unitManager.AddUnit(Unit(grid.ToWorldPosition(grid.GetTiles(TileType::MayorHouse)[0]) + Vector2F{ Random::Range(0, 25), Random::Range(0, 25) }));
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

		if (tile.NeedToBeDestroyed)
		{
			// Cancel the destruction
			tile.NeedToBeDestroyed = false;
			tile.Progress = 0;
		}
		// Can be destroyed immediately
		else if (tile.Type == TileType::Road || !tile.IsBuilt)
		{
			tile.Type = tile.Type == TileType::Quarry ? TileType::Stone : TileType::None;
			tile.IsBuilt = true;
			tile.NeedToBeDestroyed = false;
			tile.Progress = 0;
		}
		// Can be destroyed by a builder
		else if (grid.CanBeDestroyed(tilePosition))
		{
			tile.Progress = 0;
			tile.NeedToBeDestroyed = true;
		}
	}

	if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_RIGHT))
	{
		// Remove some tiles with permissions and other are set to be destroyed by a builder
		auto mousePosition = Input::GetMousePosition();
		auto mouseWorldPosition = Window::ScreenToWorld(mousePosition);
		auto tilePosition = grid.GetTilePosition(mouseWorldPosition);
		auto& tile = grid.GetTile(tilePosition);

		if (tile.Type != TileType::Road) return;

		tile.Type = TileType::None;
		tile.IsBuilt = true;
		tile.NeedToBeDestroyed = false;
		tile.Progress = 0;
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
		case TileType::LogisticsCenter:
			selectedTileTexture = Texture(Buildings::LogisticsCenter);
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

	auto logisticsCenter = Tile(TileType::LogisticsCenter);
	logisticsCenter.IsBuilt = true;
	grid.SetTile(grid.GetTilePosition(centerOfScreen) + TilePosition{ 0, 2 }, logisticsCenter);

	auto house = Tile(TileType::House);
	house.IsBuilt = true;
	grid.SetTile(grid.GetTilePosition(centerOfScreen) + TilePosition{ 2, 2 }, house);

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
			if (Random::Range(0, 100) < 1)
			{
				tile.Type = TileType::Stone;
				tile.IsBuilt = true;
			}
		}
	});

	Random::StopUseSeed();

	for (int i = 0; i < 3; i++)
	{
		unitManager.AddUnit(Unit(grid.ToWorldPosition(grid.GetTiles(TileType::MayorHouse)[0]) + Vector2F{ Random::Range(0, 25), Random::Range(0, 25) }));
	}
}