#pragma once

#include <vector>

#include "Maths.h"
#include "Color.h"
#include "Texture.h"
#include "DrawableObject.h"

struct Vertex
{
    Vector2F Position = {0, 0};
    Color Color { 1.f, 1.f, 1.f, 1.f };
    float U = 0, V = 0;
};

struct Camera
{
    Vector2F Position = { 0, 0 };
    float Zoom = 1.f;
};

namespace Window
{
    int GetFrameCount();

	/**
	 * @brief Convert a position (0, 0 to screen width, screen height) to world space (-1, -1 to 1, 1)
	 * @param position A position in the range (0, 0 to screen width, screen height)
	 * @return A position in the range (-1, -1 to 1, 1)
	 */
	Vector2F ToWorldSpace(Vector2F position);

	/**
	 * @brief Convert a position (-1, -1 to 1, 1) to world space (0, 0 to screen width, screen height)
	 * @param position A position in the range (-1, -1 to 1, 1)
	 * @return A position in the range (0, 0 to screen width, screen height)
	 */
    Vector2F ToScreenSpace(Vector2F position);

	/**
	 * @brief Convert a position given by the input system to world space (0, 0 to screen width, screen height)
	 * @param position An input position given by sokol_input
	 * @return A position in the range (0, 0 to screen width, screen height)
	 */
    Vector2F ConvertInputPosition(Vector2F position);

	/**
	 * @brief Get the UVs for a texture name
	 * @param texture The texture name
	 * @return The 4 uvs positions for the texture
	 */
    std::vector <Vector2F> GetUvs(TextureName texture);

	/**
	 * @brief Get the transformed position for the given parameters
	 * @param position The original position to transform
	 * @param pivot The pivot point to rotate around (0, 0 to 1, 1)
	 * @param scale The scale to apply to the position
	 * @param rotationDegree The rotation to apply to the position in degrees
	 * @param size The size of the object
	 * @return The transformed position
	 */
    Vector2F GetTransformedPosition(Vector2F position, Vector2F pivot, Vector2F scale, float rotationDegree, Vector2F size);

    void AppendVertex(Vertex vertex);

	/**
	 * @brief Draw a rectangle
	 * @param position The position in the screen space, starting from the top left corner
	 * @param size The size of the rectangle
	 * @param color The color of the rectangle
	 * @param uvs Needed if using a texture
	 */
    void DrawRect(Vector2F position, Vector2F size, Color color, std::vector <Vector2F> uvs = {});

	/**
	 * @brief Draw an object according to its properties
	 * @param object The object to draw
	 */
    void DrawObject(DrawableObject object);

	/**
	 * @brief Move the camera by the given amount
	 * @param position The amount to move the camera by in screen space
	 */
    void MoveCamera(Vector2F position);

	/**
	 * @brief Zoom the camera by the given amount (1 is no zoom)
	 * @param scale The amount to zoom the camera by
	 */
    void Zoom(float scale);
}