#include <cmath>

#include "iostream"


#include "Audio.h"
#include "Window.h"
#include "Input.h"
#include "sokol_app.h"
#include "Timer.h"

DrawableObject movableObject = 
{
    .Position = { 700, 700 },
    .Pivot = { 0.5f, 0.5f },
    .Size = { 100, 100 },
    .UseTexture = true,
    .TextureName = TextureName::Top
};

DrawableObject player =
{
    .Position = { 300, 300 },
    .Pivot = { 0.5f, 0.5f },
    .Size = { 50, 50 },
    .UseTexture = true,
    .TextureName = TextureName::CenterLeft
};

float speed = 200.f;

void InitGame()
{
    std::cout << "Start Function" << std::endl;

    // TODO : Correct the link problems in the audio file.

    // Audio::SetupSound();

    // SoundClip testTheme = Audio::loadSoundClip("assets/testTheme.wav");

    // Audio::PlaySoundClip(testTheme, 1.f, 440, 0, 0, true);
}

void OnFrame()
{
    auto mousePosition = Input::GetMousePosition();
    auto previousMousePosition = Input::GetPreviousMousePosition();
    auto smoothDeltaTime = Timer::SmoothDeltaTime;
    auto movementValue = speed * smoothDeltaTime;

    if (Input::IsKeyHeld(SAPP_KEYCODE_A))
    {
        player.Position.X -= movementValue;
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_D))
    {
        player.Position.X += movementValue;
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_W))
    {
        player.Position.Y += movementValue;
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_S))
    {
        player.Position.Y -= movementValue;
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

    auto value = std::abs(std::sin(Timer::Time));

    movableObject.Scale = { value, value };

    Window::DrawObject(movableObject);
    Window::DrawObject(player);
}