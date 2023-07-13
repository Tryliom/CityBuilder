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

    // Random::SetSeed(42);
    // Random::UseSeed();

    // for (Tile &tile : road.Tiles)
    // {
    //     tile.Texture = Texture((Land)Random::Range(0, (int)Land::Count - 1));
    // }

    // Random::StopUseSeed();
    // Matrix2x3F mutlipled = Matrix2x3F::Multiply(rot,tr);
    // std::cout << Matrix2x3F::Multiply(mutlipled,myVec).X  << " " << Matrix2x3F::Multiply(mutlipled,myVec).Y << std::endl;
}

void OnFrame()
{
    auto mousePosition = Input::GetMousePosition();

    UpdateCamera();

    grid.Update();
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

    if (Input::IsMouseButtonHeld(SAPP_MOUSEBUTTON_MIDDLE))
    {
        Window::MoveCamera((mousePosition - previousMousePosition) * Vector2F{1, -1} * 1.f / Window::GetZoom());
    }

    if (Input::IsMouseButtonPressed(SAPP_MOUSEBUTTON_LEFT))
    {
        grid.SetTile(grid.GetTilePosition(mousePosition), Tile(Texture(Ressources::TreeSprout)));
    }
    if (Input::IsMouseButtonPressed(SAPP_MOUSEBUTTON_RIGHT))
    {
        grid.RemoveTile(grid.GetTilePosition(mousePosition));
    }


}