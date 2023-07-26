#include "Graphics.h"
#include "Input.h"
#include "Logger.h"

#include <cassert>
#include <iostream>

constexpr float textureBleed = 0.005f;

namespace Graphics
{
    int GetFrameCount()
    {
        return Graphics::frameCount;
    }

	void IncreaseFrameCount()
	{
		Graphics::frameCount++;
	}

    std::vector<Vector2F> GetUvs(Texture texture)
    {
        int tileSheetIndex = static_cast<int>(texture.TileSheetIndex);
        int textureSize = tileSheets[tileSheetIndex].GetHeight();
        int tileMapY = 0;
		float widthPercent = textureSize / (float) textureWidth;
		float heightPercent = textureSize / (float) textureHeight;

        for (int i = 0; i < tileSheetIndex; i++)
        {
            tileMapY += tileSheets[i].GetHeight() + 1;
        }

        float width = widthPercent * (1.f - textureBleed);
        float height = heightPercent * (1.f - textureBleed);
        float X = texture.TileIndex * textureSize / (float)textureWidth + widthPercent * textureBleed;
        float Y = tileMapY / (float)textureHeight + heightPercent * textureBleed;

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
		vertexes[vertexIndex + 9] = (float) frameCount;

        vertexesUsed++;
    }

    void DrawRect(Vector2F position, Vector2F size, Color color, std::vector<Vector2F> uvs)
    {
		if (!IsVisible(position, size)) return;

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

        for (int i = 0; i < (int) points.size() - 2; i++)
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
		const float width = camera.ScreenSize.X;
		const float height = camera.ScreenSize.Y;
		const Vector2F MaxSize = { 2500, 2500};
		const Vector2F MinSize = { -2500, -2500};
		const Vector2F transformedMaxSize = Matrix2x3F::Multiply(transformMatrix, MaxSize);
		const Vector2F transformedMinSize = Matrix2x3F::Multiply(transformMatrix, MinSize);

		const Vector2I maxPosition = { transformedMaxSize.X, transformedMaxSize.Y };
		const Vector2I minPosition = { transformedMinSize.X, transformedMinSize.Y };

		bool clamped = false;

		// Get a world position and check where it's located in the screen space
		if (0 < minPosition.X)
		{
			camera.Position.X = -MinSize.X;
			clamped = true;
		}
		else if (width > maxPosition.X)
		{
			camera.Position.X = -MaxSize.X + width;
			clamped = true;
		}

		if (0 < minPosition.Y)
		{
			camera.Position.Y = -MinSize.Y;
			clamped = true;
		}
		else if (height > maxPosition.Y)
		{
			camera.Position.Y = -MaxSize.Y + height;
			clamped = true;
		}

		if (!clamped)
		{
			camera.Position += position;
		}
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

	void SetCameraSize(float width, float height)
	{
		camera.ScreenSize = { width, height };
	}

    void CalculTransformationMatrix(Vector2F scale)
    {
        transformMatrix     = Matrix2x3F::TransformMatrix(scale, 0, camera.Position, camera.Pivot);
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

		memset(vertexes, 0, sizeof(vertexes));
		memset(indices, 0, sizeof(indices));
    }

	bool IsVisible(Vector2F position, Vector2F size)
	{
		Vector2F transformedPosition = Matrix2x3F::Multiply(transformMatrix, position);
		Vector2F transformedSize = size;
		float width = camera.ScreenSize.X;
		float height = camera.ScreenSize.Y;

		// If only one of the 4 corners is visible or the camera is inside, the object is visible
		return
			(transformedPosition.X >= 0 && transformedPosition.X <= width && transformedPosition.Y >= 0 && transformedPosition.Y <= height) ||
			(transformedPosition.X + transformedSize.X >= 0 && transformedPosition.X + transformedSize.X <= width && transformedPosition.Y >= 0 && transformedPosition.Y <= height) ||
			(transformedPosition.X >= 0 && transformedPosition.X <= width && transformedPosition.Y + transformedSize.Y >= 0 && transformedPosition.Y + transformedSize.Y <= height) ||
			(transformedPosition.X + transformedSize.X >= 0 && transformedPosition.X + transformedSize.X <= width && transformedPosition.Y + transformedSize.Y >= 0 && transformedPosition.Y + transformedSize.Y <= height) ||
			(transformedPosition.X <= 0 && transformedPosition.X + transformedSize.X >= width && transformedPosition.Y <= 0 && transformedPosition.Y + transformedSize.Y >= height);
	}
}

void Serialize(Serializer* ser, Camera* camera)
{
    Serialize(ser, &camera->Position);
    Serialize(ser, &camera->Zoom);
}