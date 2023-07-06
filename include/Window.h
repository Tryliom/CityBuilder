#pragma once

#include <vector>
#include "Maths.h"
#include "Color.h"
#include "Texture.h"
#include "DrawableObject.h"

struct Vertex
{
    Vector2F Position = {0, 0};
    Color Color{ 1.f, 1.f, 1.f, 1.f };
    float U = 0, V = 0;
};

struct Camera
{
    Vector2F Position = {0, 0};
    float Zoom = 1.f;
};

namespace Window
{
    int GetFrameCount();
    Vector2F ToScreenSpace(Vector2F position);
    Vector2F ToWorldSpace(Vector2F position);
    Vector2F ConvertInputPosition(Vector2F position);
    std::vector <Vector2F> GetUvs(TextureName texture);
    Vector2F GetScaledPosition(Vector2F position, Vector2F pivot, Vector2F scale, float rotationDegree, Vector2F size);

    void AppendVertex(Vertex vertex);

    void DrawRect(Vector2F position, Vector2F size, Color color, std::vector <Vector2F> uvs = {});
    void DrawObject(DrawableObject object);

    void MoveCamera(Vector2F position);
    void Zoom(float scale);
}