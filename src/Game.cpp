#include <cmath>
#include <iostream>

#include "sokol_app.h"

#include "Window.h"
#include "Audio.h"
#include "Input.h"
#include "Timer.h"

void UpdateCamera();

float speed = 500.f;

Grid grid(5000, 5000, 100);

void InitGame()
{
    std::cout << "Start Function" << std::endl;

    /*Audio::SetupSound();

    SoundClip testTheme = Audio::loadSoundClip(ASSETS_PATH "testTheme.wav");*/

    // Audio::PlaySoundClip(testTheme, 1.f, 440, 0, 0, true);

    Vector2F myVec(3,4);
    Matrix2x3F id =  Matrix2x3F::IdentityMatrix();
    Matrix2x3F tr =  Matrix2x3F::TranslationMatrix({15,10});
    Matrix2x3F rot = Matrix2x3F::RotationMatrix(90);
    Matrix2x3F sc  = Matrix2x3F::ScaleMatrix({2, 2});

    std::cout << Matrix2x3F::Multiply(id,myVec).X  << " " << Matrix2x3F::Multiply(id,myVec).Y  << std::endl;
    std::cout << Matrix2x3F::Multiply(tr,myVec).X  << " " << Matrix2x3F::Multiply(tr,myVec).Y  << std::endl;
    std::cout << Matrix2x3F::Multiply(rot,myVec).X << " " << Matrix2x3F::Multiply(rot,myVec).Y << std::endl;
    std::cout << Matrix2x3F::Multiply(sc,myVec).X  << " " << Matrix2x3F::Multiply(sc,myVec).Y  << std::endl;

    Matrix2x3F mutlipled = Matrix2x3F::Multiply(rot,tr);
    std::cout << Matrix2x3F::Multiply(mutlipled,myVec).X  << " " << Matrix2x3F::Multiply(mutlipled,myVec).Y << std::endl;
}

void OnFrame()
{
    auto mousePosition = Input::GetMousePosition();

    UpdateCamera();

    grid.Draw();
}

void UpdateCamera()
{
    auto mousePosition = Input::GetMousePosition();
    auto previousMousePosition = Input::GetPreviousMousePosition();
    auto smoothDeltaTime = Timer::SmoothDeltaTime;
    auto movementValue = speed * smoothDeltaTime;

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
        Window::MoveCamera({0, -movementValue});
    }
    if (Input::IsKeyHeld(SAPP_KEYCODE_S))
    {
        Window::MoveCamera({0, movementValue});
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

    if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_LEFT))
    {
        Window::MoveCamera((mousePosition - previousMousePosition) * Vector2F{1, -1} * 1.f / Window::GetZoom());
    }
}