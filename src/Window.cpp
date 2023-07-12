#include "Window.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "basic-sapp.glsl.h"

#include "Constants.h"
#include "Input.h"
#include "Image.h"
#include "Logger.h"
#include "Timer.h"

struct vs_window
{
    float width;
    float height;
};

static struct {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    vs_window vs_window;
} state;

#pragma region Attributes

// ===== Sokol =====

const int VertexNbAttributes = 9;

float vertexes[10000];
int vertexesUsed = 0;

uint32_t indices[10000];
int indicesUsed = 0;

// ===== Window =====

int frameCount = 0;
int textureWidth = 0;
int textureHeight = 0;

// Textures
std::vector<Texture> textures = {};

Camera camera;

#pragma endregion

#pragma region SokolFunctions

void InitGame();

void OnFrame();

void Clear()
{
    vertexesUsed = 0;
    indicesUsed = 0;
}

static void init()
{
    InitGame();

    sg_desc desc = (sg_desc){
            .logger = { .func = slog_func },
            .context = sapp_sgcontext()
    };
    sg_setup(desc);

    state.vs_window = { sapp_widthf(), sapp_heightf() };

    state.bind.vertex_buffers[0] = sg_make_buffer((sg_buffer_desc){
            .size = sizeof(vertexes),
            .usage = SG_USAGE_DYNAMIC,
            .label = "triangle-vertices",
    });

    state.bind.index_buffer = sg_make_buffer((sg_buffer_desc){
            .size = sizeof(indices),
            .type = SG_BUFFERTYPE_INDEXBUFFER,
            .usage = SG_USAGE_DYNAMIC,
            .label = "triangle-indices",
    });

    Image image(ASSETS_PATH "road.png");

    textureWidth = image.GetWidth();
    textureHeight = image.GetHeight();

    state.bind.fs_images[SLOT_tex] = sg_make_image((sg_image_desc)
    {
           .width = textureWidth,
           .height = textureHeight,
           .data = {.subimage = {{{ .ptr = image.GetBuffer(), .size = image.GetBufferSize() }}}},
           .label = "test-image"
    });

	for (auto i = 0; i < textureWidth / textureHeight; i++)
	{
		textures.push_back(Texture(i * textureHeight, 0, textureHeight, textureHeight));
	}

    // Create shader from code-generated sg_shader_desc
    sg_shader shd = sg_make_shader(ui_shader_desc(sg_query_backend()));

    // Create a pipeline object (default render states are fine for triangle)
    sg_pipeline_desc pip_desc = {
            .shader = shd,
            // If the vertex layout doesn't have gaps, don't need to provide strides and offsets
            .layout =
            {
                .attrs =
                {
                    { .format = SG_VERTEXFORMAT_FLOAT3 },
                    { .format = SG_VERTEXFORMAT_FLOAT4 },
                    { .format = SG_VERTEXFORMAT_FLOAT2 }
                }
            },
            .colors =
            {
                {
                    // Set up the alpha blending
                    .blend =
                    {
                        .enabled = true,
                        .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                        .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                        .op_rgb = SG_BLENDOP_ADD,
                        .src_factor_alpha = SG_BLENDFACTOR_ONE,
                        .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                        .op_alpha = SG_BLENDOP_ADD
                    }
                }
            },
            .index_type = SG_INDEXTYPE_UINT32,
            .label = "triangle-pipeline",
    };
    state.pip = sg_make_pipeline(pip_desc);

    // a pass action to clear framebuffer to black
    state.pass_action = (sg_pass_action)
    {
        .colors = { { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f } } }
    };

	int width = sapp_width();
	int height = sapp_height();

	camera.Position = { (float) width, (float) height};
}

void frame()
{
    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);

    auto width = sapp_width();
    auto height = sapp_height();

    state.vs_window = { (float) width, (float) height };

    // Clear buffers
    Clear();

    Input::Update();
    Timer::Update();

    frameCount++;
    OnFrame();

    sg_update_buffer(state.bind.vertex_buffers[0], (sg_range) { .ptr = vertexes, .size = vertexesUsed * VertexNbAttributes * sizeof(*vertexes) });
    sg_update_buffer(state.bind.index_buffer, (sg_range) { .ptr = indices, .size = indicesUsed * sizeof(*indices) });

    sg_draw(0, vertexesUsed * VertexNbAttributes, 1);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, (sg_range) { .ptr = &state.vs_window, .size = sizeof(state.vs_window) * 2 });
    sg_end_pass();
    sg_commit();
}

void cleanup()
{
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    return (sapp_desc)
	{
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = Input::OnInput,
        .width = 640,
        .height = 640,
        .window_title = "City builder",
        .logger = { .func = slog_func },
        .win32_console_create = true // Use it if you want to see console output
    };
}

#pragma endregion

namespace Window
{
    int GetFrameCount()
    {
        return frameCount;
    }

    Vector2F ConvertInputPosition(Vector2F position)
    {
        return { position.X, sapp_height() - position.Y };
    }

    std::vector <Vector2F> GetUvs(TextureName texture)
    {
        float width = textures[static_cast<int>(texture)].Width / (float) textureWidth;
        float height = textures[static_cast<int>(texture)].Height / (float) textureHeight;
        float X = textures[static_cast<int>(texture)].X / (float) textureWidth;
        float Y = textures[static_cast<int>(texture)].Y / (float) textureHeight;

        return {
                { X,         Y },
                { X + width, Y },
                { X + width, Y + height },
                { X,         Y + height }
        };
    }

    Vector2F GetTransformedPosition(Vector2F position, Vector2F pivot, Vector2F scale, float rotationDegree, Vector2F size)
    {
        //TODO: Implement rotation

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
        int vertexIndex = vertexesUsed * VertexNbAttributes;

        vertexes[vertexIndex] = (vertex.Position.X + camera.Position.X) * camera.Zoom;
        vertexes[vertexIndex + 1] = (vertex.Position.Y + camera.Position.Y) * camera.Zoom;
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
            uvs = {{ -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }};
        }

        int startIndex = vertexesUsed;

        AppendVertex({ position, color, uvs[0].X, uvs[0].Y });
        AppendVertex({ position + Vector2F{ size.X, 0 }, color, uvs[1].X, uvs[1].Y });
        AppendVertex({ position + size, color, uvs[2].X, uvs[2].Y });
        AppendVertex({ position + Vector2F{ 0, size.Y }, color, uvs[3].X, uvs[3].Y });

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
			float angle = (float) i / (float) segments * 2.f * 3.1415926f;

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

		for (auto& point : points)
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

        if (object.UseTexture)
        {
            uvs = GetUvs(object.TextureName);
        }

        Vector2F position = GetTransformedPosition(object.Position, object.Pivot, object.Scale, object.Rotation, object.Size);

        DrawRect(position, object.Size * object.Scale, object.Color, uvs);
    }

	void DrawGrid(const Grid& grid)
	{
		float width = sapp_widthf();
		float height = sapp_heightf();

		for (Tile tile : grid.Tiles)
		{
			DrawObject({
				.Position = tile.Position - Vector2F{width / 2, height / 2},
				.Size = Vector2F{(float) grid.TileSize, (float) grid.TileSize},
				.Color = tile.Color,
				.UseTexture = tile.TextureName != TextureName::None,
				.TextureName = tile.TextureName,
			});
		}
	}

    void MoveCamera(Vector2F position)
    {
        camera.Position += position;
    }

    void Zoom(float scale)
    {
        camera.Zoom += scale;

        if (camera.Zoom < 0.f)
        {
            camera.Zoom = 0.f;
        }
    }
}