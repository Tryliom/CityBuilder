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
    float pad0, pad1;
};

static struct
{
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    vs_window vs_window;
} state;

#pragma region Attributes

// ===== Sokol =====

const int VertexNbAttributes = 9;

const int maxVertexes = 100000;
float vertexes[maxVertexes];
int vertexesUsed = 0;

uint32_t indices[maxVertexes];
int indicesUsed = 0;

// ===== Window =====

int frameCount = 0;
int textureWidth = 0;
int textureHeight = 0;

// Textures
std::vector<Image> tileSheets =
    {
        Image(ASSETS_PATH "land.png"),
        Image(ASSETS_PATH "road.png"),
        Image(ASSETS_PATH "buildings.png"),
        Image(ASSETS_PATH "ressources.png"),
        Image(ASSETS_PATH "characters.png"),
        Image(ASSETS_PATH "icons.png")};

Camera camera;

Matrix2x3F transformMatrix;
Matrix2x3F inversedTransMatrix;

constexpr float MinZoom = 1.f;
constexpr float MaxZoom = 2.f;

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
        .logger = {.func = slog_func},
        .context = sapp_sgcontext()};
    sg_setup(desc);

    state.vs_window = {sapp_widthf(), sapp_heightf()};

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

    Image tileMap;

    tileMap.AddImagesAtRow(tileSheets);

    textureWidth = tileMap.GetWidth();
    textureHeight = tileMap.GetHeight();

    state.bind.fs_images[SLOT_tex] = sg_make_image((sg_image_desc){
        .width = textureWidth,
        .height = textureHeight,
        .data = {.subimage = {{{.ptr = tileMap.GetBuffer(), .size = tileMap.GetBufferSize()}}}},
        .label = "tilemap-image"});

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
                        {.format = SG_VERTEXFORMAT_FLOAT3},
                        {.format = SG_VERTEXFORMAT_FLOAT4},
                        {.format = SG_VERTEXFORMAT_FLOAT2}}},
        .colors =
            {
                {// Set up the alpha blending
                 .blend =
                     {
                         .enabled = true,
                         .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                         .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                         .op_rgb = SG_BLENDOP_ADD,
                         .src_factor_alpha = SG_BLENDFACTOR_ONE,
                         .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                         .op_alpha = SG_BLENDOP_ADD}}},
        .index_type = SG_INDEXTYPE_UINT32,
        .label = "triangle-pipeline",
    };
    state.pip = sg_make_pipeline(pip_desc);

    // a pass action to clear framebuffer to black
    state.pass_action = (sg_pass_action){
        .colors = {{.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.0f, 0.0f, 0.0f, 1.0f}}}};
}

void frame()
{
    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);

    auto width = sapp_width();
    auto height = sapp_height();

    state.vs_window = {(float)width, (float)height};

    // Clear buffers
    Clear();

    Input::Update();
    Timer::Update();

    frameCount++;
    OnFrame();

    sg_update_buffer(state.bind.vertex_buffers[0], (sg_range){.ptr = vertexes, .size = vertexesUsed * VertexNbAttributes * sizeof(*vertexes)});
    sg_update_buffer(state.bind.index_buffer, (sg_range){.ptr = indices, .size = indicesUsed * sizeof(*indices)});

    sg_draw(0, vertexesUsed * VertexNbAttributes, 1);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, (sg_range){.ptr = &state.vs_window, .size = sizeof(state.vs_window)});
    sg_end_pass();
    sg_commit();
}

void cleanup()
{
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = Input::OnInput,
        .width = 640,
        .height = 640,
        .window_title = "City builder",
        .logger = {.func = slog_func},
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

    std::vector<Vector2F> GetUvs(Texture texture)
    {
        int tileSheetIndex = static_cast<int>(texture.TileSheetIndex);
        int textureSize = tileSheets[tileSheetIndex].GetHeight();
        int tileMapY = 0;

        for (int i = 0; i < tileSheetIndex; i++)
        {
            tileMapY += tileSheets[i].GetHeight();
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
        transformMatrix     = Matrix2x3F::TransformMatrix({camera.Zoom, camera.Zoom}, 0, camera.Position);
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
}