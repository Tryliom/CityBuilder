#include <cmath>
#include <iostream>

#include "sokol_app.h"

#include "Audio.h"
#include "Input.h"
#include "Timer.h"
#include "Unit.h"
#include "Random.h"
#include "Logger.h"
#include "UnitManager.h"

#include "Graphics.h"

#include <assert.h>

void GenerateMap();

void UpdateCamera();
void HandleInput();
void DrawUi();

float speed = 500.f;

struct GameState
{
	Camera Camera;

	Grid Grid;
	UnitManager UnitManager;

	TileType SelectedTileType;
};

GameState* gameState = nullptr;

bool newDLL = false;

void InitGame()
{
    /*Audio::SetupSound();

    SoundClip testTheme = Audio::loadSoundClip(ASSETS_PATH "testTheme.wav");*/

    // Audio::PlaySoundClip(testTheme, 1.f, 440, 0, 0, true);

	GenerateMap();

	// tilemap->AddImagesAtRow(Graphics::tileSheets);

	// Graphics::textureWidth  = tilemap->GetWidth();
	// Graphics::textureHeight = tilemap->GetHeight();
}

void OnFrame()
{
	Input::Update();
	Graphics::ClearFrameBuffers();

	auto mousePosition = Input::GetMousePosition();

	UpdateCamera();
	HandleInput();

	Graphics::CalculTransformationMatrix();

	gameState->Grid.Update();
	gameState->Grid.Draw();

	gameState->UnitManager.UpdateUnits();
	gameState->UnitManager.DrawUnits();

	DrawUi();
}

#ifdef __cplusplus // If used by C++ code, 
extern "C"         // we need to export the C interface
{          
    #if _WIN32
    #define EXPORT __declspec(dllexport)
    #else
    #define EXPORT
    #endif

	EXPORT void DLL_OnInput(const sapp_event* event)
	{
		Input::OnInput(event);
	}

	EXPORT void DLL_InitGame(void* gameMemory, Image* tilemap)
	{
		gameState = (GameState*)gameMemory;

		gameState->Camera = Graphics::camera;

		gameState->Grid = Grid(5000, 5000, 100);
		gameState->UnitManager.SetGrid(&gameState->Grid);
		gameState->SelectedTileType = TileType::Sawmill;
	
		InitGame();

		tilemap->AddImagesAtRow(Graphics::tileSheets);

		Graphics::textureWidth  = tilemap->GetWidth();
		Graphics::textureHeight = tilemap->GetHeight();
	}

    EXPORT void DLL_OnFrame(void* gameMemory, FrameData* frameData, TimerData* timerData)
	{
		gameState = (GameState*)gameMemory;

		if (Graphics::textureWidth == 0)
		{
			Graphics::camera = gameState->Camera;

			Image tilemap;

			tilemap.AddImagesAtRow(Graphics::tileSheets);

			Graphics::textureWidth  = tilemap.GetWidth();
			Graphics::textureHeight = tilemap.GetHeight();
		}

		OnFrame();

		Graphics::DrawRect(Vector2F(100, 100), Vector2F(200, 200), Color::Red);

		gameState->Camera = Graphics::camera;

		frameData->vertexBufferPtr  = Graphics::vertexes;
		frameData->vertexBufferUsed = Graphics::vertexesUsed;
		frameData->indexBufferPtr   = Graphics::indices;
		frameData->indexBufferUsed  = Graphics::indicesUsed;

		Timer::Time = timerData->Time;
		Timer::DeltaTime = timerData->DeltaTime;
		Timer::SmoothDeltaTime = timerData->SmoothDeltaTime;
	}
}
#endif

void UpdateCamera()
{
    auto mousePosition = Input::GetMousePosition();
    auto previousMousePosition = Input::GetPreviousMousePosition();
    auto smoothDeltaTime = Timer::SmoothDeltaTime;
    auto movementValue = speed * smoothDeltaTime;
    auto mouseWorldPosition = Graphics::ScreenToWorld(mousePosition);

    if (Input::IsKeyHeld(SAPP_KEYCODE_A))
    {
        Graphics::MoveCamera({movementValue, 0});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_D))
    {
        Graphics::MoveCamera({-movementValue, 0});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_W))
    {
        Graphics::MoveCamera({0, movementValue});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_S))
    {
        Graphics::MoveCamera({0, -movementValue});
    }

    if (Input::IsKeyPressed(SAPP_KEYCODE_Z))
    {
        Graphics::Zoom(0.1f);
    }
    if (Input::IsKeyPressed(SAPP_KEYCODE_X))
    {
        Graphics::Zoom(-0.1f);
    }

    Graphics::Zoom(Input::GetMouseWheelDelta() / 50.f);

    if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_MIDDLE))
	{
		Graphics::MoveCamera((mousePosition - previousMousePosition) * 1.f / Graphics::GetZoom());
	}
}

void HandleInput()
{
	if (Input::IsKeyPressed(SAPP_KEYCODE_1))
	{
		gameState->SelectedTileType = TileType::Sawmill;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_2))
	{
		gameState->SelectedTileType = TileType::BuilderHut;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_3))
	{
		gameState->SelectedTileType = TileType::Quarry;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_4))
	{
		gameState->SelectedTileType = TileType::Storage;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_5))
	{
		gameState->SelectedTileType = TileType::House;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_6))
	{
		gameState->SelectedTileType = TileType::Road;
	}
	if (Input::IsKeyPressed(SAPP_KEYCODE_7))
	{
		gameState->SelectedTileType = TileType::LogisticsCenter;
	}

	if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_LEFT))
	{
		auto mousePosition = Input::GetMousePosition();
		auto mouseWorldPosition = Graphics::ScreenToWorld(mousePosition);
		auto tilePosition = gameState->Grid.GetTilePosition(mouseWorldPosition);

		if (gameState->Grid.CanBuild(tilePosition, gameState->SelectedTileType))
		{
			gameState->Grid.SetTile(tilePosition, Tile(gameState->SelectedTileType));
		}
	}

	if (Input::IsMouseButtonPressed(SAPP_MOUSEBUTTON_RIGHT))
	{
		// Remove some tiles with permissions and other are set to be destroyed by a builder
		auto mousePosition = Input::GetMousePosition();
		auto mouseWorldPosition = Graphics::ScreenToWorld(mousePosition);
		auto tilePosition = gameState->Grid.GetTilePosition(mouseWorldPosition);
		auto& tile = gameState->Grid.GetTile(tilePosition);

		if (tile.Type == TileType::None) return;

		if (tile.NeedToBeDestroyed)
		{
			// Cancel the destruction
			tile.NeedToBeDestroyed = false;
			tile.Progress = 0;
			return;
		}

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
		if (tile.Type == TileType::Stone || tile.Type == TileType::Tree || tile.Type == TileType::Storage || tile.Type == TileType::House || tile.Type == TileType::BuilderHut
			|| tile.Type == TileType::Sawmill || tile.Type == TileType::Quarry || tile.Type == TileType::LogisticsCenter)
		{
			tile.Progress = 0;
			tile.NeedToBeDestroyed = true;
		}
	}
}

void DrawUi()
{
	Texture selectedTileTexture;

	switch (gameState->SelectedTileType)
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
	Graphics::DrawRect(Graphics::ScreenToWorld({5, 5}), {110, 110}, {0.2f, 0.2f, 0.2f, 0.5f});
	Graphics::DrawObject({
		.Position = Graphics::ScreenToWorld({10, 10}),
		.Size = {100, 100},
		.Texture = selectedTileTexture,
	});
}

void GenerateMap()
{
	Vector2F centerOfScreen = Vector2F{ sapp_widthf(), sapp_heightf() } / 2.f;

	auto mayorHouse = Tile(TileType::MayorHouse);
	mayorHouse.IsBuilt = true;
	gameState->Grid.SetTile(gameState->Grid.GetTilePosition(centerOfScreen), mayorHouse);

	auto builderHouse = Tile(TileType::BuilderHut);
	builderHouse.IsBuilt = true;
	gameState->Grid.SetTile(gameState->Grid.GetTilePosition(centerOfScreen) + TilePosition{ 2, 0}, builderHouse);

	auto logisticsCenter = Tile(TileType::LogisticsCenter);
	logisticsCenter.IsBuilt = true;
	gameState->Grid.SetTile(gameState->Grid.GetTilePosition(centerOfScreen) + TilePosition{ 0, 2 }, logisticsCenter);

	// Generate a random seed for the map
	Random::SetSeed(Random::Range(0, 1000000));
	Random::UseSeed();

	// Place random trees
	gameState->Grid.ForEachTile([&](Tile& tile, TilePosition position)
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
	gameState->Grid.ForEachTile([&](Tile& tile, TilePosition position)
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
		gameState->UnitManager.AddUnit(Unit{.Position = gameState->Grid.ToWorldPosition(
			gameState->Grid.GetTiles(TileType::MayorHouse)[0]) + Vector2F{ Random::Range(0, 25), Random::Range(0, 25) }});
	}
}