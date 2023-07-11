#include <cmath>
#include <iostream>

#include "sokol_app.h"

#include "Window.h"
#include "Audio.h"
#include "Input.h"
#include "Timer.h"
#include "Constants.h"

void UpdateCamera();
void UpdateGrid();

float speed = 200.f;

Grid road(800, 800, 50);

void InitGame()
{
    std::cout << "Start Function" << std::endl;

    Audio::SetupSound();

    SoundClip testTheme = Audio::loadSoundClip(ASSETS_PATH "testTheme.wav");

    // Audio::PlaySoundClip(testTheme, 1.f, 440, 0, 0, true);

	for (Tile& tile : road.Tiles)
	{
		tile.TextureName = TextureName::SingleRoad;
	}
}

void OnFrame()
{
    auto mousePosition = Input::GetMousePosition();

    UpdateCamera();

	UpdateGrid();

	Window::DrawGrid(road);
}

void UpdateGrid()
{
	auto mousePosition = Input::GetMousePosition();

	for (Tile& tile : road.Tiles)
	{
		if (tile.Position.X < mousePosition.X && tile.Position.X + tile.Size.X > mousePosition.X &&
			tile.Position.Y < mousePosition.Y && tile.Position.Y + tile.Size.Y > mousePosition.Y)
		{
			tile.SetSelected(true);
		}
		else
		{
			tile.SetSelected(false);
		}
	}
}

void UpdateCamera()
{
    auto mousePosition = Input::GetMousePosition();
    auto previousMousePosition = Input::GetPreviousMousePosition();
    auto smoothDeltaTime = Timer::SmoothDeltaTime;
    auto movementValue = speed * smoothDeltaTime;

    if (Input::IsKeyHeld(SAPP_KEYCODE_A))
    {
        Window::MoveCamera({-movementValue, 0});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_D))
    {
        Window::MoveCamera({movementValue, 0});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_W))
    {
        Window::MoveCamera({0, movementValue});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_S))
    {
        Window::MoveCamera({0, -movementValue});
    }

    if (Input::IsKeyHeld(SAPP_KEYCODE_Z))
    {
        Window::Zoom(0.01f);
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_X))
    {
        Window::Zoom(-0.01f);
    }

    if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_LEFT))
    {
        Window::MoveCamera(mousePosition - previousMousePosition);
    }
}