#include <cmath>
#include <iostream>

#include "sokol_app.h"

#include "Window.h"
#include "Audio.h"
#include "Input.h"
#include "Timer.h"
#include "Constants.h"

void UpdateCamera();

DrawableObject movableObject =
    {
        .Position = {700, 700},
        .Pivot = {0.5f, 0.5f},
        .Size = {100, 100},
        .UseTexture = true,
        .TextureName = TextureName::Top};

DrawableObject player =
    {
        .Position = {300, 300},
        .Pivot = {0.5f, 0.5f},
        .Size = {50, 50},
        .Color = Color(1.f, 0.f, 0.f, 1.f),
        .UseTexture = true,
        .TextureName = TextureName::CenterLeft};

float speed = 200.f;

void InitGame()
{
    std::cout << "Start Function" << std::endl;

    Audio::SetupSound();

    SoundClip testTheme = Audio::loadSoundClip(ASSETS_PATH "testTheme.wav");

    // Audio::PlaySoundClip(testTheme, 1.f, 440, 0, 0, true);
}

void OnFrame()
{
    auto mousePosition = Input::GetMousePosition();

    UpdateCamera();

    auto value = std::abs(std::sin(Timer::Time));
    auto uiMousePosition = Window::ToUiSpace(mousePosition);

    movableObject.Scale = {value, value};

    Window::DrawObject(movableObject);
    Window::DrawObject(player);

    Window::DrawLine(player.Position, uiMousePosition, 5.f, Color(1.f, 1.f, 1.f, 0.5f));
    Window::DrawCircle(uiMousePosition, 5.f, Color(0.f, 1.f, 0.f, 0.2f));
    Window::DrawCustomShape({{0, 0}, {100, 100}, {200, 0}}, Color(1.f, 0.f, 0.f, 0.5f));
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