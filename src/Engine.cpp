#define SOKOL_IMPL
//#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "util/sokol_imgui.h"

#include "basic-sapp.glsl.h"

#include "Audio.h"
#include "Constants.h"
#include "Image.h"
#include "Input.h"
#include "Logger.h"
#include "Timer.h"

#include "Graphics.h"

#include <assert.h>
#ifdef _WIN32
    #include <malloc.h> // NOTE: on unix there is no malloc.h, it's in stdlib or something
    #include <Windows.h> 
    #include <Shlwapi.h> // Include this header for PathCombine function
    #pragma comment(lib, "Shlwapi.lib") // Link to the Shlwapi library
#endif

#ifdef __APPLE__
    #include <dlfcn.h>
#endif

#include <stdint.h>
#include <stdio.h>


struct vs_window
{
    float width;
    float height;
    float frame, pad1;
};

static struct
{
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    vs_window vs_window;
} state;

// ====== Imgui =========

static bool show_test_window = true;
static bool show_another_window = false;

#pragma region Attributes

// ====== Hot Reload =========

static void (*DLL_OnLoad)  (Image*, FrameData*, ImGuiData*, ImTextureID*) = nullptr;
static void (*DLL_OnInput) (const sapp_event*) = nullptr;
static void (*DLL_InitGame)(void*, Image*, FrameData*, ImGuiData*, ImTextureID*) = nullptr;
static void (*DLL_OnFrame) (void*, FrameData*, TimerData*, const simgui_frame_desc_t*) = nullptr;

void* gameStateMemory = nullptr;

// Handle to a loaded module (DLL).
void* libHandle = NULL; 

int64_t lastMod = 0;

Image tilemap;
ImTextureID imTextureID;
ImGuiData imguiData = {};
FrameData frameData = {};
TimerData timerData = {};

auto mainTheme = Audio::loadSoundClip("assets/mainTheme.wav");

void InitGame(void* gameMemory, Image* tilemap, FrameData* frameData, ImGuiData* engineImGuiData, ImTextureID* imTextureID);

void OnFrame(FrameData* frameData, TimerData* timerData, const simgui_frame_desc_t* simgui_frame_desc);

void RunnerOnEvent(const sapp_event* event)
{
    if (DLL_OnInput) DLL_OnInput(event);
    else Input::OnInput(event);

    simgui_handle_event(event);
}


#ifdef _WIN32

bool FileExists(const char* path) 
{
	return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

int64_t LastModified(const char* path) 
{
	if (!FileExists(path)) 
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

#else

#include <sys/stat.h> // for mkdir
#include <sys/types.h>

int64_t LastModified(const char* path) 
{
	struct stat info;
	int err = stat(path, &info);
	assert(err == 0);

    //struct timespec time = info.st_mtimespec;
	time_t time = info.st_mtime; // From some documentation: "For historical reasons, it is generally implemented as an integral value representing the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC(i.e., a unix timestamp). Although libraries may implement this type using alternative time representations.

	return (uint64_t) time;
}

void* PlatformDllOpen(const char* path)
{
    #if defined(_WIN32)
        HINSTANCE hInst;
        hInst = LoadLibrary(path);
        if (hInst==NULL) {
            printf("rdx_dll_open error: %s\n", GetLastError());
            exit(-1);
        }
        return hInst;
	#elif defined(__APPLE__) || defined(LINUX)
        void* lib = dlopen(path, RTLD_LOCAL | RTLD_NOW);
        if (lib == NULL) {
            char buff[1000];
            getcwd(buff, 1000);
            printf("Couldn't load dynamic lib '%s': %s\n", path, dlerror());
            abort();
        }
        return lib;
	#endif
}

#define MAX_PATH 256 // TEMP

#endif

int PlatformDllClose(void* handle)
{
	#ifdef _WIN32
    int rc = 0;
	BOOL ok;
    ok = FreeLibrary((HINSTANCE)handle);
    if (!ok) {
		printf("PlatformDllClose error: %s\n", GetLastError());
        //var.lasterror = GetLastError();
        //var.err_rutin = "dlclose";
        rc = -1;
    }
    return rc;
	#elif defined(__APPLE__) || defined(LINUX)
	return dlclose(handle);
	#endif
}

void* PlatformGetSymbol(void* handle, const char* name)
{
	#ifdef _WIN32
	FARPROC fp;

    fp = GetProcAddress((HINSTANCE)handle, name);
    if (!fp) {
		printf("PlatformGetSymbol error: %s\n", GetLastError());
    }
    return (void *)(intptr_t)fp;
	#elif defined(__APPLE__) || defined(LINUX)
	return dlsym(handle, name);
	#endif
}

void LoadDLL()
{
    int64_t newLastMod = LastModified("bin/Game.dll");

    if (newLastMod != -1 && newLastMod != lastMod) 
    {
        LOG("NEW DLL LOAD");

        lastMod = newLastMod;

        // Unload the previous DLL if it was loaded
        if (libHandle != NULL)
        {
            PlatformDllClose(libHandle);
            libHandle = NULL; // Reset the handle to indicate that the DLL is no longer loaded
        }

        // Copy Game.dll to a random filename in the same directory before loading
        #ifdef _WIN32
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
        #else
            char* newDllPath = (char*) "./newGame.dylib";
        #endif

        // Load new version of the lib : could ask the game to deserialize itself just after loading).
        libHandle = PlatformDllOpen(newDllPath);

        assert(libHandle != NULL && "Couldn't load Game.dll");

        DLL_OnLoad = (void (*)(Image*, FrameData*, ImGuiData*, ImTextureID*))PlatformGetSymbol(libHandle, "DLL_OnLoad"); 
        assert(DLL_OnLoad != NULL && "Couldn't find function DLL_OnLoad in Game.dll");

        DLL_OnInput = (void (*)(const sapp_event*))PlatformGetSymbol(libHandle, "DLL_OnInput"); 
        assert(DLL_OnInput != NULL && "Couldn't find function DLL_OnInput in Game.dll");

        DLL_InitGame = (void (*)(void*, Image*, FrameData*, ImGuiData*, ImTextureID*))PlatformGetSymbol(libHandle, "DLL_InitGame"); 
        assert(DLL_InitGame != NULL && "Couldn't find function DLL_InitGame in Game.dll");

        DLL_OnFrame  = (void (*)(void*, FrameData*, TimerData*, const simgui_frame_desc_t*))PlatformGetSymbol(libHandle, "DLL_OnFrame"); 
        assert(DLL_OnFrame != NULL && "Couldn't find function DLL_OnFrame in Game.dll");

        if (DLL_OnLoad) DLL_OnLoad(&tilemap, &frameData, &imguiData, &imTextureID);
    }
}

// =========================================

#pragma endregion

#pragma region SokolFunctions

static void init()
{ 
    frameData.screenSize   = Vector2F{sapp_widthf(), sapp_heightf()};
    frameData.screenCenter = Vector2F{sapp_widthf(), sapp_heightf()} / 2.f;

    //sapp_toggle_fullscreen();

	tilemap.AddImagesAtRow(Graphics::tileSheets);

    sg_desc desc = (sg_desc){
        .logger = {.func = slog_func},
        .context = sapp_sgcontext()};
    sg_setup(desc);

    state.vs_window = {sapp_widthf(), sapp_heightf(), 0};

    state.bind.vertex_buffers[0] = sg_make_buffer((sg_buffer_desc){
        .size = Graphics::maxVertexes * sizeof(frameData.vertexBufferPtr[0]),
        .usage = SG_USAGE_DYNAMIC,
        .label = "triangle-vertices",
    });

    state.bind.index_buffer = sg_make_buffer((sg_buffer_desc){
        .size = Graphics::maxVertexes * sizeof(frameData.indexBufferPtr[0]),
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .usage = SG_USAGE_DYNAMIC,
        .label = "triangle-indices",
    });

    state.bind.fs_images[SLOT_tex] = sg_make_image((sg_image_desc){
        .width  = tilemap.GetWidth(),
        .height = tilemap.GetHeight(),
        .data = {.subimage = {{{.ptr = tilemap.GetBuffer(), .size = tilemap.GetBufferSize()}}}},
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
                {.format = SG_VERTEXFORMAT_FLOAT2},
				{.format = SG_VERTEXFORMAT_FLOAT},
			}
		},
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
                     .op_alpha = SG_BLENDOP_ADD
				 }
			}
		},
        .index_type = SG_INDEXTYPE_UINT32,
        .label = "triangle-pipeline",
    };
    state.pip = sg_make_pipeline(pip_desc);

    // use sokol-imgui with all default-options (we're not doing
    // multi-sampled rendering or using non-default pixel formats)
    simgui_desc_t simgui_desc = { };
    simgui_setup(&simgui_desc);

    imguiData.Context = ImGui::GetCurrentContext();
    imguiData.Context->IO.DeltaTime = sapp_frame_duration();
    
    imguiData.IO = &ImGui::GetIO();

    gameStateMemory = malloc(GAME_STATE_MAX_BYTE_SIZE);
    memset(gameStateMemory, 0, GAME_STATE_MAX_BYTE_SIZE);

    imTextureID = (ImTextureID)(uintptr_t)state.bind.fs_images[SLOT_tex].id;

    #ifdef HOT_RELOAD
    LoadDLL();
    if(DLL_InitGame) DLL_InitGame(gameStateMemory, &tilemap, &frameData, &imguiData, &imTextureID);
    #else
    InitGame(gameStateMemory, &tilemap, &frameData, &imguiData, &imTextureID);
    #endif

    Audio::SetupSound();
	Audio::PlaySoundClip(mainTheme, 0.5f, 440, 0, 0, true);

    // a pass action to clear framebuffer to green
    state.pass_action = (sg_pass_action)
	{
        .colors = {{.load_action = SG_LOADACTION_CLEAR, .clear_value = {95 / 255.f, 195 / 255.f, 65 / 255.f, 1.f}}}
	};
}

void frame()
{
	Graphics::IncreaseFrameCount();

    auto width  = sapp_width();
    auto height = sapp_height();

    state.vs_window = {(float)width, (float)height, (float) Graphics::GetFrameCount()};

    frameData.screenSize   = Vector2I{sapp_width(), sapp_height()};
    frameData.screenCenter = Vector2I{sapp_width(), sapp_height()} / 2;

    frameData.frameCount = Graphics::GetFrameCount();

	//Graphics::SetCameraSize(frameData.screenSize.X, frameData.screenSize.Y);

    Timer::Update();
    timerData.Time = Timer::Time;
    timerData.DeltaTime = Timer::DeltaTime;
    timerData.SmoothDeltaTime = Timer::SmoothDeltaTime;

    const simgui_frame_desc_t imguiframeDesc{ width, height, sapp_frame_duration(), sapp_dpi_scale() };

    #ifdef HOT_RELOAD
    LoadDLL();
    if (DLL_OnFrame) DLL_OnFrame(gameStateMemory, &frameData, &timerData, &imguiframeDesc);
    #else
    
    OnFrame(&frameData, &timerData, &imguiframeDesc);
    #endif

    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pip);
    
    // Check that the indices are in bounds
    // NOTE(seb): added this when debugging the graphic issue. Leaving it as it can't hurt!
    #ifndef NDEBUG
    for (int index_idx = 0; index_idx < frameData.indexBufferUsed; index_idx++) {
        int idx = frameData.indexBufferPtr[index_idx];
        assert(idx < frameData.vertexBufferUsed && "An index was bigger than the index of the last vertex.");
        assert(idx >= 0 && "An index was smaller than zero.");
    }
    #endif

    sg_update_buffer(state.bind.vertex_buffers[0], (sg_range){
		.ptr = frameData.vertexBufferPtr,
		.size = frameData.vertexBufferUsed * Graphics::VertexNbAttributes * sizeof(frameData.vertexBufferPtr[0])
	});

    sg_update_buffer(state.bind.index_buffer, (sg_range){
        .ptr = frameData.indexBufferPtr,
        .size = frameData.indexBufferUsed * sizeof(frameData.indexBufferPtr[0])
    });

    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_window, (sg_range){.ptr = &state.vs_window, .size = sizeof(state.vs_window)});
    sg_apply_bindings(&state.bind);

    //sg_draw(0, frameData.vertexBufferUsed * Graphics::VertexNbAttributes, 1);
    sg_draw(0, frameData.indexBufferUsed, 1);

	simgui_render();
    sg_end_pass();
    sg_commit();
}

void cleanup()
{
    //free(gameStateMemory);
    simgui_shutdown();
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
        .event_cb = RunnerOnEvent,
        .width = 1000,
        .height = 800,
        .window_title = "City builder",
        .logger = {.func = slog_func},
        .win32_console_create = true // Use it if you want to see console output
    };
}

#pragma endregion