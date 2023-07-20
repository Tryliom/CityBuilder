#pragma once 

#include "Maths.h"
#include "Color.h"
#include "Image.h"
#include "Texture.h"
#include "DrawableObject.h"
#include "Constants.h"
#include <vector>

struct Vertex
{
    Vector2F Position = {0, 0};
    Color Color { 1.f, 1.f, 1.f, 1.f };
    float U = -1, V = -1; // -1 means no texture
};

struct Camera
{
    Vector2F Position = { 0, 0 };
    float Zoom = 1.f;
	Vector2F Pivot;
};

struct FrameData
{
	float* vertexBufferPtr;
	int vertexBufferUsed;
	uint32_t* indexBufferPtr;
	int indexBufferUsed;

	Vector2F screenCenter;
};

struct TextureData
{
	std::vector<Image> tileSheets;

    int frameCount;
    int textureWidth;
    int textureHeight;
};

namespace Graphics
{
    inline const int VertexNbAttributes = 9;

    inline const int maxVertexes = 1000000;
    inline float vertexes[maxVertexes];
    inline int vertexesUsed = 0;

    inline uint32_t indices[maxVertexes];
    inline int indicesUsed = 0;

    inline Camera camera;

    inline float MinZoom = 0.7f;
    inline float MaxZoom = 2.f;

    inline Matrix2x3F transformMatrix;
    inline Matrix2x3F inversedTransMatrix;

    // Textures
    inline std::vector<Image> tileSheets =
    {
        Image(ASSETS_PATH "land.png"),
        Image(ASSETS_PATH "road.png"),
        Image(ASSETS_PATH "buildings.png"),
        Image(ASSETS_PATH "ressources.png"),
        Image(ASSETS_PATH "characters.png"),
        Image(ASSETS_PATH "icons.png")
    };

    inline int frameCount = 0;
    inline int textureWidth = 0;
    inline int textureHeight = 0;

    int GetFrameCount();

    /**
	 * @brief Get the UVs for a texture name
	 * @param texture The texture
	 * @return The 4 uvs positions for the texture
	 */
    std::vector <Vector2F> GetUvs(Texture texture);

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
	 * @brief Draw a circle
	 * @param position The position in the screen space, starting from center of the circle
	 * @param radius The radius of the circle
	 * @param color The color of the circle
	 * @param segments The number of segments to use to draw the circle
	 */
	void DrawCircle(Vector2F position, float radius, Color color, int segments = 50);

	/**
	 * @brief Draw a line
	 * @param start The start position of the line
	 * @param end The end position of the line
	 * @param thickness The thickness of the line
	 * @param color The color of the line
	 */
	void DrawLine(Vector2F start, Vector2F end, float thickness, Color color);

	/**
	 * @brief Draw a custom shape using the given points, starting from the first point
	 * @param points The points to draw the shape with
	 * @param color The color of the shape
	 */
	void DrawCustomShape(std::vector<Vector2F> points, Color color);

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

    float GetZoom();

	void CalculTransformationMatrix();

    Vector2F ScreenToWorld(Vector2F vec);

    // temp

    int GetTextureWidth();
    int GetTextureHeight();

    size_t GetVertexBufferSize();
    size_t GetIndexBufferSize();

	void ClearFrameBuffers();
}
