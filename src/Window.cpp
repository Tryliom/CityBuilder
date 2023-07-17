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

// ====== Hot Reload =========

#include <assert.h>
#include <malloc.h>
#include <Windows.h> 
#include <stdint.h>
#include <stdio.h>
#include <Shlwapi.h> // Include this header for PathCombine function

#pragma comment(lib, "Shlwapi.lib") // Link to the Shlwapi library

bool RdxFileExists(const char* path) 
{
	return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

int64_t RdxLastModified(const char* path) 
{
	if (!RdxFileExists(path)) 
    {
		return -1;
	}

	FILETIME ftLastWriteTime;

    // Open the file, specifying the desired access rights, sharing mode, and other parameters.
	HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE && GetFileTime(hFile, NULL, NULL, &ftLastWriteTime)) 
    {
		ULARGE_INTEGER uli;
		uli.LowPart  = ftLastWriteTime.dwLowDateTime;
		uli.HighPart = ftLastWriteTime.dwHighDateTime;
		CloseHandle(hFile);

		return static_cast<int64_t>(uli.QuadPart / 10000000ULL - 11644473600ULL);
	}

	return -1;
}

#define GAME_STATE_MAX_BYTE_SIZE 4096

void (*DLL_InitGame)() = nullptr;
void (*DLL_OnFrame)() = nullptr;
void (*DLL_SetGraphicBufferValues)(float*, int&, uint32_t*, int&) = nullptr;

void* gameStateMemory = nullptr;

HMODULE libHandle = NULL; // Handle to a loaded module (DLL).

uint64_t lastMod = 0;

void LoadDLL()
{
    uint64_t newLastMod = RdxLastModified("bin/Game.dll");

    std::cout <<" new " << newLastMod << " last " << lastMod << std::endl;

    if (newLastMod != -1 && newLastMod != lastMod) 
    {
        lastMod = newLastMod;

        if (libHandle != NULL) 
        {
            FreeLibrary(libHandle);
        }

        // Copy Game.dll to a random filename in the same directory before loading
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        char dllPath[MAX_PATH];
        strcpy_s(dllPath, sizeof(dllPath), exePath);
        PathRemoveFileSpecA(dllPath); // Remove the executable filename from the path
        PathAppendA(dllPath, "Game.dll");

        char newDllPath[MAX_PATH];
        strcpy_s(newDllPath, sizeof(newDllPath), exePath);
        PathRemoveFileSpecA(newDllPath); // Remove the executable filename from the path
        PathAppendA(newDllPath, "newGame.dll");

        CopyFileA(dllPath, newDllPath, FALSE);

        // Load new version of the lib (NOTE(seb): could ask the game to deserialize itself just after loading).
        libHandle = LoadLibraryA(newDllPath);

        assert(libHandle != NULL && "Couldn't load Game.dll");

        DLL_InitGame = (void (*)())GetProcAddress(libHandle, "DLL_InitGame"); 
        assert(DLL_InitGame != NULL && "Couldn't find function DLL_InitGame in Game.dll");

        DLL_OnFrame = (void (*)())GetProcAddress(libHandle, "DLL_OnFrame"); 
        assert(DLL_OnFrame != NULL && "Couldn't find function DLL_OnFrame in Game.dll");

        DLL_SetGraphicBufferValues = (void (*)(float*, int&, uint32_t*, int&))GetProcAddress(libHandle, "DLL_SetGraphicBufferValues"); 
        assert(DLL_SetGraphicBufferValues != NULL && "Couldn't find function DLL_SetGraphicBufferValues in Game.dll");
    }
}

// =========================================

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
    #ifdef HOT_RELOAD
    LoadDLL();
    #endif

    if (DLL_InitGame) DLL_InitGame();
    else InitGame();

    sg_desc desc = (sg_desc){
        .logger = {.func = slog_func},
        .context = sapp_sgcontext()};
    sg_setup(desc);

    state.vs_window = {sapp_widthf(), sapp_heightf()};

    state.bind.vertex_buffers[0] = sg_make_buffer((sg_buffer_desc){
        .size = Graphics::GetVertexBufferSize(),
        .usage = SG_USAGE_DYNAMIC,
        .label = "triangle-vertices",
    });

    state.bind.index_buffer = sg_make_buffer((sg_buffer_desc){
        .size = Graphics::GetIndexBufferSize(),
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .usage = SG_USAGE_DYNAMIC,
        .label = "triangle-indices",
    });

    Image tileMap;

    tileMap.AddImagesAtRow(Graphics::tileSheets);

    Graphics::textureWidth = tileMap.GetWidth();
    Graphics::textureHeight = tileMap.GetHeight();

    state.bind.fs_images[SLOT_tex] = sg_make_image((sg_image_desc){
        .width  = Graphics::textureWidth,
        .height = Graphics::textureHeight,
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
    #ifdef HOT_RELOAD
    LoadDLL();
    #endif

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

    if (DLL_OnFrame) DLL_OnFrame();
    else OnFrame();

    if (DLL_SetGraphicBufferValues)
    {
        DLL_SetGraphicBufferValues(Graphics::vertexes, Graphics::vertexesUsed, Graphics::indices, Graphics::indicesUsed);
    } 

    std::cout << "Used " << Graphics::vertexesUsed << std::endl;

    sg_update_buffer(state.bind.vertex_buffers[0], (sg_range){.ptr = Graphics::vertexes, 
                     .size = Graphics::vertexesUsed * Graphics::VertexNbAttributes * sizeof(*Graphics::vertexes)});
    sg_update_buffer(state.bind.index_buffer, (sg_range){.ptr = Graphics::indices, .size = Graphics::indicesUsed * sizeof(*Graphics::indices)});

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