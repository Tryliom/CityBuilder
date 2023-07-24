#include "sokol_app.h"

#include "Audio.h"
#include "Input.h"
#include "Timer.h"
#include "Unit.h"
#include "Random.h"
#include "Logger.h"
#include "UnitManager.h"
#include "Grid.h"
#include "Logger.h"

#include "Graphics.h"
#include "GUI.h"

#include "sokol_gfx.h"
#include "imgui.h"
#include "util\sokol_imgui.h"

// ========= Game Initialization functions ===========

void GenerateMap();

// ========= Game Update functions ===========

void UpdateCamera();
void HandleInput();
void DrawUi();

// ========= Data exchange functions ===========

void BindWithEngine(Image* tilemap, FrameData* frameData, ImGuiData* engineImGuiData, ImTextureID* imTextureID);
void ReceiveDataFromEngine(FrameData* frameData, TimerData* timerData);
void SendDataToEngine(FrameData* frameData);

// ========= Game attributes ===========

struct GameState
{
	Camera Camera;

	Grid Grid;
	UnitManager UnitManager;

	bool GameStarted = false;
	bool GamePaused  = false;

	// int Seed;
};

GameState *gameState = nullptr;

TileType textureToTileType[] =
{
	TileType::Sawmill,
	TileType::BuilderHut,
	TileType::Quarry,
	TileType::Storage,
	TileType::House,
	TileType::Road,
	TileType::LogisticsCenter,
	TileType::Furnace
};

int buildingSelected = 0;

int gridWidth = 5000, gridHeight = 5000, tileSize = 100;

Vector2F screenSize;
Vector2F centerOfScreen;

float speed = 500.f;
float sprintSpeed = 800.f;

ImGuiData currentImGuiData = {};

// The mouse position with the world matrix applied to.
Vector2F mousePositionInWorld;
bool isMouseOnAWindow;

ImTextureID* imTilemapTextureID;

// =========== Game Logic ============

void InitGame(void* gameMemory, Image* tilemap, FrameData* frameData, ImGuiData* engineImGuiData, ImTextureID* imTextureID)
{
	BindWithEngine(tilemap, frameData, engineImGuiData, imTextureID);

	gameState = (GameState *)gameMemory;

	gameState->Grid = Grid(gridWidth, gridHeight, tileSize);
	gameState->UnitManager.SetGrid(&gameState->Grid);

	GenerateMap();

	gameState->Camera.Zoom = 1.f;
}

void OnFrame(FrameData *frameData, TimerData *timerData, const simgui_frame_desc_t* simguiFrameDesc)
{
	ReceiveDataFromEngine(frameData, timerData);

	Graphics::ClearFrameBuffers();

	simgui_new_frame(simguiFrameDesc);

	Graphics::SetCameraSize(frameData->screenSize.X, frameData->screenSize.Y);
	UpdateCamera();
	// Update the current camera state.
	Graphics::camera.Pivot = centerOfScreen;
	gameState->Camera = Graphics::camera;
	Graphics::CalculTransformationMatrix();

	auto mousePosition = Input::GetMousePosition();
	isMouseOnAWindow = currentImGuiData.IO->WantCaptureMouse;

	HandleInput();

	Input::Update();

	if (!gameState->GameStarted)
	{
		GUI::DrawStartMenu(&gameState->GameStarted);
	}
	
	gameState->Grid.Update();
	gameState->UnitManager.UpdateUnits();

	gameState->Grid.Draw(true, isMouseOnAWindow);
	gameState->UnitManager.DrawUnits(true);
	gameState->Grid.Draw(false, isMouseOnAWindow);
	gameState->UnitManager.DrawUnits(false);

	// Reset the transformation matrix in order to not apply the world transformation to the UI.
	Graphics::CalculTransformationMatrix(Vector2F::One);
	DrawUi();

	// Show the ImGui test window. Most of the sample code is in ImGui::ShowDemoWindow()
	ImGui::SetNextWindowPos(ImVec2(460, 20), ImGuiCond_FirstUseEver);
	ImGui::ShowDemoWindow();

	SendDataToEngine(frameData);
}

void UpdateCamera()
{
	if (!gameState->GameStarted) return;

	auto mousePosition = Input::GetMousePosition();
	auto previousMousePosition = Input::GetPreviousMousePosition();
	auto smoothDeltaTime = Timer::SmoothDeltaTime;
	auto movementValue = Input::IsKeyHeld(SAPP_KEYCODE_LEFT_SHIFT) ? sprintSpeed * smoothDeltaTime : speed * smoothDeltaTime;
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

	if (!isMouseOnAWindow)
	{
		float mouseWheelDelta = Input::GetMouseWheelDelta();

		Graphics::Zoom(mouseWheelDelta / 50.f);

		if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_MIDDLE) || Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_LEFT) && buildingSelected == -1)
		{
			Graphics::MoveCamera((mousePosition - previousMousePosition) * 1.f / Graphics::GetZoom());
		}
	}
}

void HandleInput()
{
	if (Input::IsKeyPressed(SAPP_KEYCODE_ESCAPE))
	{
		gameState->GamePaused = !gameState->GamePaused;
	}

	mousePositionInWorld = Graphics::ScreenToWorld(Input::GetMousePosition());

	if (Input::IsKeyPressed(SAPP_KEYCODE_F))
	{
		// Spawn a unit
		gameState->UnitManager.AddUnit(Unit(gameState->Grid.ToWorldPosition(gameState->Grid.GetTiles(TileType::MayorHouse)[0]) + Vector2F{Random::Range(0, 25), Random::Range(0, 25)}));
	}

	if (!isMouseOnAWindow)
	{
		if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_LEFT) && buildingSelected != -1)
		{
			auto mousePosition = Input::GetMousePosition();
			auto mouseWorldPosition = Graphics::ScreenToWorld(mousePosition);
			auto tilePosition = gameState->Grid.GetTilePosition(mouseWorldPosition);

			if (gameState->Grid.IsTileValid(tilePosition) && gameState->Grid.CanBuild(tilePosition, textureToTileType[buildingSelected]))
			{
				gameState->Grid.SetTile(tilePosition, Tile(textureToTileType[buildingSelected]));

				if (Input::IsKeyHeld(SAPP_KEYCODE_LEFT_SHIFT))
				{
					gameState->Grid.GetTile(tilePosition).IsBuilt = true;
				}
			}
		}

		if (Input::IsMouseButtonPressed(SAPP_MOUSEBUTTON_RIGHT))
		{
			// Remove some tiles with permissions and other are set to be destroyed by a builder
			auto mousePosition = Input::GetMousePosition();
			auto mouseWorldPosition = Graphics::ScreenToWorld(mousePosition);
			auto tilePosition = gameState->Grid.GetTilePosition(mouseWorldPosition);
			auto &tile = gameState->Grid.GetTile(tilePosition);

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
			else if (gameState->Grid.CanBeDestroyed(tilePosition))
			{
				tile.Progress = 0;
				tile.NeedToBeDestroyed = true;
			}
		}

		if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_RIGHT))
		{
			// Remove some tiles with permissions and other are set to be destroyed by a builder
			auto mousePosition = Input::GetMousePosition();
			auto mouseWorldPosition = Graphics::ScreenToWorld(mousePosition);
			auto tilePosition = gameState->Grid.GetTilePosition(mouseWorldPosition);
			auto &tile = gameState->Grid.GetTile(tilePosition);

			if (tile.Type != TileType::Road)
				return;

			tile.Type = TileType::None;
			tile.IsBuilt = true;
			tile.NeedToBeDestroyed = false;
			tile.Progress = 0;
		}
	}
}

void DrawUi()
{
	if (gameState->GamePaused)
	{
		GUI::DrawPauseMenu(&gameState->GamePaused);
	}

	GUI::DrawConstructionMenu(&buildingSelected, &screenSize, imTilemapTextureID);

	TilePosition mouseTilePosition = gameState->Grid.GetTilePosition(mousePositionInWorld);

	if (gameState->Grid.IsTileValid(mouseTilePosition))
	{
		Tile &tile = gameState->Grid.GetTile(mouseTilePosition);

        if (tile.Type != TileType::None && tile.Type != TileType::Road)
        {
			GUI::DrawTileInventory(tile, &isMouseOnAWindow);
        }
    }
}

void GenerateMap()
{
	auto mayorHouse = Tile(TileType::MayorHouse);
	mayorHouse.IsBuilt = true;
	gameState->Grid.SetTile(gameState->Grid.GetTilePosition(centerOfScreen), mayorHouse);

	auto builderHouse = Tile(TileType::BuilderHut);
	builderHouse.IsBuilt = true;
	gameState->Grid.SetTile(gameState->Grid.GetTilePosition(centerOfScreen) + TilePosition{2, 0}, builderHouse);

	auto logisticsCenter = Tile(TileType::LogisticsCenter);
	logisticsCenter.IsBuilt = true;
	gameState->Grid.SetTile(gameState->Grid.GetTilePosition(centerOfScreen) + TilePosition{0, 2}, logisticsCenter);

	auto house = Tile(TileType::House);
	house.IsBuilt = true;
	gameState->Grid.SetTile(gameState->Grid.GetTilePosition(centerOfScreen) + TilePosition{2, 2}, house);

	// Generate a random seed for the map
	Random::SetSeed(Random::Range(0, 1000000));
	Random::UseSeed();

	// Place random trees
	gameState->Grid.ForEachTile([&](Tile &tile, TilePosition position)
								{
		if (tile.Type == TileType::None)
		{
			if (Random::Range(0, 100) < 10)
			{
				tile.Type = TileType::Tree;
				tile.TreeGrowth = Random::Range(0.f, 30.f);
				tile.IsBuilt = true;
			}
		} });

	// Place random rocks
	gameState->Grid.ForEachTile([&](Tile &tile, TilePosition position)
								{
		if (tile.Type == TileType::None)
		{
			if (Random::Range(0, 100) < 1)
			{
				tile.Type = TileType::Stone;
				tile.IsBuilt = true;
			}
		} });

	Random::StopUseSeed();

	for (int i = 0; i < 3; i++)
	{
		gameState->UnitManager.AddUnit(Unit(gameState->Grid.ToWorldPosition(gameState->Grid.GetTiles(TileType::MayorHouse)[0]) + Vector2F{Random::Range(0, 25), Random::Range(0, 25)}));
	}
}

void BindWithEngine(Image* tilemap, FrameData* frameData, ImGuiData* engineImGuiData, ImTextureID* imTextureID)
{
	screenSize     = frameData->screenSize;
	centerOfScreen = frameData->screenCenter;

	currentImGuiData.Context = engineImGuiData->Context;
	currentImGuiData.IO      = engineImGuiData->IO;

	ImGui::SetCurrentContext(engineImGuiData->Context);

	Graphics::textureWidth  = tilemap->GetWidth();
	Graphics::textureHeight = tilemap->GetHeight();

	imTilemapTextureID = imTextureID;
}

void ReceiveDataFromEngine(FrameData* frameData, TimerData* timerData)
{
	// Update the screen values.
	screenSize     = frameData->screenSize;
	centerOfScreen = frameData->screenCenter;

	Graphics::frameCount = frameData->frameCount;

	// Get the timer data from the engine.
	Timer::Time = timerData->Time;
	Timer::DeltaTime = timerData->DeltaTime;
	Timer::SmoothDeltaTime = timerData->SmoothDeltaTime;
}

void SendDataToEngine(FrameData* frameData)
{
	// Send the frame data to the engine.
	frameData->vertexBufferPtr  = Graphics::vertexes;
	frameData->vertexBufferUsed = Graphics::vertexesUsed;
	frameData->indexBufferPtr   = Graphics::indices;
	frameData->indexBufferUsed  = Graphics::indicesUsed;
}

#ifdef __cplusplus // If used by C++ code,
extern "C"		   // we need to export the C interface
{
#if _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

	EXPORT void DLL_OnInput(const sapp_event *event)
	{
		Input::OnInput(event);
	}

	EXPORT void DLL_OnLoad(Image* tilemap, FrameData* frameData, ImGuiData* engineImGuiData, ImTextureID* imTextureID)
	{
		BindWithEngine(tilemap, frameData, engineImGuiData, imTextureID);
	}

	EXPORT void DLL_InitGame(void* gameMemory, Image* tilemap, FrameData* frameData, ImGuiData* engineImGuiData, ImTextureID* imTextureID)
	{
		InitGame(gameMemory, tilemap, frameData, engineImGuiData, imTextureID);
	}

	EXPORT void DLL_OnFrame(void *gameMemory, FrameData *frameData, TimerData *timerData, const simgui_frame_desc_t* simguiFrameDesc)
	{
		// Update the gameState. 
		// When a new DLL is created, it will automatically set his gameState to the old one.
		gameState = (GameState *)gameMemory;

		Graphics::camera = gameState->Camera;

		OnFrame(frameData, timerData, simguiFrameDesc);
	}
}
#endif