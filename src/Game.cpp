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

	// int Seed;
};

GameState *gameState = nullptr;

Texture buildings[] =
{
	Texture(Buildings::Sawmill),
	Texture(Buildings::BuilderHut),
	Texture(Buildings::Quarry),
	Texture(Buildings::Storage),
	Texture(Buildings::House),
	Texture(Road::Single),
	Texture(Buildings::LogisticsCenter),
	Texture(Buildings::InactiveFurnace)
};

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

bool isButtonSelected[ARR_LEN(buildings)] =
{
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false
};

int gridWidth = 5000, gridHeight = 5000, tileSize = 100;

Vector2F screenSize;
Vector2F centerOfScreen;

float speed = 500.f;

ImGuiData currentImGuiData = {};

// The mouse position with the world matrix applied to.
Vector2F mousePositionInWorld;
bool isMouseOnAWindow;

ImTextureID* imTilemapTextureID;

// =========== UI Logic ============

bool constrMenuNeverOpened = true;

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

	UpdateCamera();
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

	gameState->Grid.Draw(true);
	gameState->UnitManager.DrawUnits(true);
	gameState->Grid.Draw(false);
	gameState->UnitManager.DrawUnits(false);

	Graphics::CalculTransformationMatrix(Vector2F::One);
	DrawUi();

	// Show the ImGui test window. Most of the sample code is in ImGui::ShowDemoWindow()
	ImGui::SetNextWindowPos(ImVec2(460, 20), ImGuiCond_FirstUseEver);
	ImGui::ShowDemoWindow();

	// Update the current camera state.
	Graphics::camera.Pivot = centerOfScreen;
	gameState->Camera = Graphics::camera;

	SendDataToEngine(frameData);
}

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
	ImGuiWindowFlags constrMenuFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_None;

	ImGui::Begin("Construction Menu", NULL, constrMenuFlags);

	if (constrMenuNeverOpened) 
	{
		ImGui::SetWindowCollapsed(true);
		constrMenuNeverOpened = false;
	}

	if (ImGui::IsWindowCollapsed() && buildingSelected != -1)
	{
		isButtonSelected[buildingSelected] = false;
		buildingSelected = -1;
	}

	ImGui::SetWindowSize(ImVec2(200, screenSize.Y - 5));
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImGui::SetWindowPos(ImVec2(screenSize.X - 5 - windowSize.x, 5), ImGuiCond_Always);

	int i = 0;

	for (auto& building : buildings)
	{
		auto uvs = Graphics::GetUvs(building);

		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // For example, set the active button color to green

		// Temporarily modify the button background color to indicate the selection
		if (isButtonSelected[i])
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // For example, set the active button color to green
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.40f, 0.61f, 0.62f));
		}

		ImGui::PushID(i);

		// Center the window content using ImGui layout features
        ImVec2 windowContentRegion = ImGui::GetContentRegionAvail();
        ImVec2 buttonSize(80, 80); // Adjust button size as needed
        ImVec2 windowCenter(ImGui::GetCursorPos().x + windowContentRegion.x * 0.5f - buttonSize.x * 0.5f,
                            ImGui::GetCursorPos().y + windowContentRegion.y * 0.5f - buttonSize.y * 0.5f);

        ImGui::SetCursorPosX(windowCenter.x);

		// Use the sg_image handle (converted to ImTextureID) for the image button
		if (ImGui::ImageButton(*imTilemapTextureID, ImVec2(80, 80), ImVec2(uvs[0].X, uvs[0].Y), ImVec2(uvs[2].X, uvs[2].Y)))
		{
			isButtonSelected[buildingSelected] = false;
			buildingSelected = i;
			isButtonSelected[buildingSelected] = true;
		}

		ImGui::PopID();

		i++;
	}

	ImGui::PopStyleColor(i * 2);

	ImGui::End();

	TilePosition mouseTilePosition = gameState->Grid.GetTilePosition(mousePositionInWorld);

	if (gameState->Grid.IsTileValid(mouseTilePosition))
	{
		Tile &tile = gameState->Grid.GetTile(mouseTilePosition);

        if (tile.Type != TileType::None && tile.Type != TileType::Road)
        {
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | 
											ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
			ImGui::SetNextWindowBgAlpha(0.5); // Transparent background
			ImGui::Begin("Inventory", NULL, window_flags);
			ImGui::SetWindowPos(ImVec2(5, 120), ImGuiCond_Always);
			ImGui::SetWindowFontScale(1.35f);
			//ImGui::SetWindowSize(ImVec2(200, 400), ImGuiCond_Always);

			std::string title = "Inventory of " + TileTypeToString(tile.Type);
	        ImGui::Text("%s", title.c_str());
	        ImGui::Separator();
			
            for (auto pair: *tile.Inventory)
            {
				//TODO: Olive, the texture of the item
				auto texture = Texture((Icons) pair.first);

                std::string text = std::to_string(pair.second) + " of " + Texture::ItemToString[(int) pair.first];

                if (!tile.IsBuilt)
                {
                    text += " / " + std::to_string(Grid::GetNeededItemsToBuild(tile.Type, pair.first)) + "\n";
                }

                ImGui::Text("%s", text.c_str());
				ImGui::Separator();
            }

			ImGui::End();
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
		// Update the gameState. When a new DLL is created, it will automatically set his gameState to the old one.
		gameState = (GameState *)gameMemory;

		Graphics::camera = gameState->Camera;

		OnFrame(frameData, timerData, simguiFrameDesc);
	}
}
#endif