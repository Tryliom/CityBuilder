#include <cmath>
#include <iostream>

#include "sokol_app.h"

#include "Window.h"
#include "Audio.h"
#include "Input.h"
#include "Timer.h"
#include "Unit.h"

void UpdateCamera();
void UpdateUnits();
void DrawUnits();

float speed = 500.f;

float unitSpeed = 100.f;
int unitSize = 10;

Grid grid(5000, 5000, 100);
std::vector<Unit> units = std::vector<Unit>();

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

    TilePosition sawmillPosition = grid.GetTilePosition({0, 0});
    grid.SetTile(sawmillPosition, Tile(TileType::Sawmill));

    for (int i = 0; i < 3; i++)
    {
        units.emplace_back();
        units.back().JobTileIndex = grid.GetTileIndex(sawmillPosition);
    }
}

void OnFrame()
{
    auto mousePosition = Input::GetMousePosition();

    UpdateCamera();

    grid.Update();
    grid.Draw();

    UpdateUnits();
    DrawUnits();
}

void UpdateUnits()
{
    for (auto& unit : units)
    {
        if (unit.JobTileIndex != -1)
        {
            Tile& tile = grid.GetTile(unit.JobTileIndex);

            unit.TimeSinceLastAction += Timer::SmoothDeltaTime;

            if (tile.Type == TileType::Sawmill)
            {
                if (unit.CurrentBehavior == UnitBehavior::Idle)
                {
                    // Check if there is a full tree
                    auto treePositions = grid.GetTiles(TileType::Tree);

                    for (auto& treePosition : treePositions)
                    {
                        auto& treeTile = grid.GetTile(treePosition);

                        if (treeTile.TreeGrowth < 30.f) continue;

                        bool isTreeInUse = false;

                        for (auto& otherUnit : units)
                        {
                            if (otherUnit.CurrentBehavior == UnitBehavior::Moving || otherUnit.CurrentBehavior == UnitBehavior::Working && otherUnit.TargetTile == treePosition)
                            {
                                isTreeInUse = true;
                                break;
                            }
                        }

                        if (isTreeInUse) continue;

                        unit.TargetTile = treePosition;
                        unit.SetBehavior(UnitBehavior::Moving);
                    }

                }
                else if (unit.CurrentBehavior == UnitBehavior::Moving)
                {
                    auto targetPosition = grid.ToWorldPosition(unit.TargetTile);

                    // Make it move to his target position
                    unit.Position += (targetPosition - unit.Position).Normalized() * unitSpeed * Timer::SmoothDeltaTime;

                    // Check if it reached his target position
                    if (std::abs(unit.Position.X - targetPosition.X) < 1.f && std::abs(unit.Position.Y - targetPosition.Y) < 1.f)
                    {
                        unit.SetBehavior(UnitBehavior::Working);
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
        }
    }
}

void DrawUnits()
{
    for (auto& unit : units)
    {
        Characters character = Characters::Unemployed;

        if (unit.JobTileIndex != -1)
        {
            Tile& jobTile = grid.GetTile(unit.JobTileIndex);

            if (jobTile.Type == TileType::Sawmill)
            {
                character = Characters::Lumberjack;
            }
        }

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
        grid.SetTile(grid.GetTilePosition(mousePosition), Tile(TileType::Tree));
    }
    if (Input::IsMouseButtonPressed(SAPP_MOUSEBUTTON_RIGHT))
    {
        grid.RemoveTile(grid.GetTilePosition(mousePosition));
    }
}