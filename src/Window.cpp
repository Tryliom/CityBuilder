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

#include "Graphics.h"

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



// ===== Window =====





constexpr float MinZoom = 1.f;
constexpr float MaxZoom = 2.f;

#pragma endregion

#pragma region SokolFunctions

void InitGame();

void OnFrame();

void Clear()
{
    Graphics::vertexesUsed = 0;
    Graphics::indicesUsed = 0;
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

    tileMap.AddImagesAtRow(Graphics::tileSheets);

    Graphics::textureWidth = tileMap.GetWidth();
    Graphics::textureHeight = tileMap.GetHeight();

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
        .colors = {{.load_action = SG_LOADACTION_CLEAR, .clear_value = {95 / 255.f, 195 / 255.f, 65 / 255.f, 1.0f}}}};
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

    Graphics::frameCount++;
    OnFrame();

    sg_update_buffer(state.bind.vertex_buffers[0], (sg_range){.ptr = vertexes, .size = vertexesUsed * VertexNbAttributes * sizeof(*vertexes)});
    sg_update_buffer(state.bind.index_buffer, (sg_range){.ptr = indices, .size = indicesUsed * sizeof(*indices)});

    sg_draw(0, Graphics::vertexesUsed * Graphics::VertexNbAttributes, 1);
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
        .width = 1000,
        .height = 800,
        .window_title = "City builder",
        .logger = {.func = slog_func},
        .win32_console_create = true // Use it if you want to see console output
    };
}

#pragma endregion