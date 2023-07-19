#include "Graphics.h"
#include "Constants.h"
#include "Input.h"
#include "Logger.h"

#include <assert.h>
#include <iostream>

#include <sokol_app.h>

namespace Graphics
{
    int GetFrameCount()
    {
        return Graphics::frameCount;
    }

    std::vector<Vector2F> GetUvs(Texture texture)
    {
        int tileSheetIndex = static_cast<int>(texture.TileSheetIndex);
        int textureSize = tileSheets[tileSheetIndex].GetHeight();
        int tileMapY = 0;

        for (int i = 0; i < tileSheetIndex; i++)
        {
            tileMapY += tileSheets[i].GetHeight() + 1;
        }

        float width = textureSize / (float)textureWidth;
        float height = textureSize / (float)textureHeight;
        float X = texture.TileIndex * textureSize / (float)textureWidth;
        float Y = tileMapY / (float)textureHeight;

        return {
            {X, Y},
            {X + width, Y},
            {X + width, Y + height},
            {X, Y + height}};
    }

    Vector2F GetTransformedPosition(Vector2F position, Vector2F pivot, Vector2F scale, float rotationDegree, Vector2F size)
    {
        // TODO: Implement rotation

        // Pivot = 0 to 1
        // Top left corner is 0, 0
        // Bottom right corner is 1, 1
        // Center is 0.5, 0.5
        auto scaledSize = size * scale;
        auto scaledPivot = scaledSize * pivot;

        return position - scaledPivot;
    }

    void AppendVertex(Vertex vertex)
    {
        vertex.Position = Matrix2x3F::Multiply(transformMatrix, vertex.Position);
        int vertexIndex = vertexesUsed * VertexNbAttributes;

        assert(vertexIndex + VertexNbAttributes < maxVertexes && "Exceeded max vertexes");

        vertexes[vertexIndex + 0] = vertex.Position.X;
        vertexes[vertexIndex + 1] = vertex.Position.Y;
        vertexes[vertexIndex + 2] = 0;
        vertexes[vertexIndex + 3] = vertex.Color.R;
        vertexes[vertexIndex + 4] = vertex.Color.G;
        vertexes[vertexIndex + 5] = vertex.Color.B;
        vertexes[vertexIndex + 6] = vertex.Color.A;
        vertexes[vertexIndex + 7] = vertex.U;
        vertexes[vertexIndex + 8] = vertex.V;

        vertexesUsed++;
    }

    void DrawRect(Vector2F position, Vector2F size, Color color, std::vector<Vector2F> uvs)
    {
        if (uvs.empty())
        {
            uvs = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}};
        }

        int startIndex = vertexesUsed;

        AppendVertex({position, color, uvs[0].X, uvs[0].Y});
        AppendVertex({position + Vector2F{size.X, 0}, color, uvs[1].X, uvs[1].Y});
        AppendVertex({position + size, color, uvs[2].X, uvs[2].Y});
        AppendVertex({position + Vector2F{0, size.Y}, color, uvs[3].X, uvs[3].Y});

        indices[indicesUsed++] = startIndex;
        indices[indicesUsed++] = startIndex + 1;
        indices[indicesUsed++] = startIndex + 2;
        indices[indicesUsed++] = startIndex;
        indices[indicesUsed++] = startIndex + 2;
        indices[indicesUsed++] = startIndex + 3;
    }

    void DrawCircle(Vector2F position, float radius, Color color, int segments)
    {
        int startIndex = vertexesUsed;

        AppendVertex({{position.X, position.Y}, color});

        for (int i = 0; i <= segments; i++)
        {
            float angle = (float)i / (float)segments * 2.f * 3.1415926f;

            AppendVertex({{position.X + cosf(angle) * radius, position.Y + sinf(angle) * radius}, color});
        }

        for (int i = 0; i <= segments; i++)
        {
            indices[indicesUsed++] = startIndex + 1;
            indices[indicesUsed++] = startIndex + i + 1;
            indices[indicesUsed++] = startIndex + i + 2;
        }
    }

    void DrawLine(Vector2F start, Vector2F end, float thickness, Color color)
    {
        Vector2F direction = end - start;
        Vector2F normal = direction.Normalized();
        normal = {-normal.Y, normal.X};

        Vector2F start1 = start + normal * thickness / 2;
        Vector2F start2 = start - normal * thickness / 2;
        Vector2F end1 = end + normal * thickness / 2;
        Vector2F end2 = end - normal * thickness / 2;

        DrawCustomShape({start1, end1, end2, start2}, color);
    }

    void DrawCustomShape(std::vector<Vector2F> points, Color color)
    {
        int startIndex = vertexesUsed;

        for (auto &point : points)
        {
            AppendVertex({{point.X, point.Y}, color});
        }

        for (int i = 0; i < points.size() - 2; i++)
        {
            indices[indicesUsed++] = startIndex;
            indices[indicesUsed++] = startIndex + i + 1;
            indices[indicesUsed++] = startIndex + i + 2;
        }
    }

    void DrawObject(DrawableObject object)
    {
        std::vector<Vector2F> uvs = {};

        if (object.Texture.TileSheetIndex != TileSheet::None)
        {
            uvs = GetUvs(object.Texture);
        }

        Vector2F position = GetTransformedPosition(object.Position, object.Pivot, object.Scale, object.Rotation, object.Size);

        DrawRect(position, object.Size * object.Scale, object.Color, uvs);
    }

    void MoveCamera(Vector2F position)
    {
        camera.Position += position;
    }

    void Zoom(float scale)
    {
        camera.Zoom += scale;

        if (camera.Zoom < MinZoom)
        {
            camera.Zoom = MinZoom;
        }
        else if (camera.Zoom > MaxZoom)
        {
            camera.Zoom = MaxZoom;
        }
    }

    float GetZoom()
    {
        return camera.Zoom;
    }

    void CalculTransformationMatrix()
    {
        Vector2F centerOfScreen = Vector2F{sapp_widthf(), sapp_heightf()} / 2.f;

        transformMatrix     = Matrix2x3F::TransformMatrix({camera.Zoom, camera.Zoom}, 0, camera.Position, centerOfScreen);
        inversedTransMatrix = Matrix2x3F::Invert(transformMatrix);
    }

    Vector2F ScreenToWorld(Vector2F vec)
    {
        return Matrix2x3F::Multiply(inversedTransMatrix, vec);
    }

    int GetTextureWidth()
    {
        return textureWidth;
    }

    int GetTextureHeight()
    {
        return textureHeight;
    }

    size_t GetVertexBufferSize()
    {
        return sizeof(vertexes);
    }

    size_t GetIndexBufferSize()
    {
        return sizeof(indices);
    }

    void ClearFrameBuffers()
    {
        vertexesUsed = 0;
        indicesUsed = 0;
    }
}