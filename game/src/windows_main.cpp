#include "game.h"

#if defined LE_BUILDING_GAME
#error Error
#endif

#if OS == WINDOWS

#include <lstd/dx_graphics.h>
#include <lstd/file.h>
#include <lstd/io/fmt.h>
#include <lstd/memory/dynamic_library.h>
#include <lstd/os.h>

#undef MAC
#undef _MAC
#include <Windows.h>
#define VREFRESH 116

static dynamic_library g_GameCode;
static game_update_and_render_func *g_GameUpdateAndRender = null;

// @TODO: This fails in Dist configuration for some reason
void reload_game_code(file::path dllPath) {
    g_GameCode.close();

    auto dllHandle = file::handle(dllPath);

    file::path copyPath = dllHandle.Path.directory();
    copyPath.combine_with("loaded_game_code.dll");

    auto dllCopyHandle = file::handle(copyPath);
    assert(dllHandle.copy(dllCopyHandle, true));

    if (!g_GameCode.load(copyPath.UnifiedPath)) {
        fmt::print("Error: Couldn't load {} (copied from {}) as the game code for the engine\n", copyPath, dllPath);
        assert(false);
    }

    g_GameUpdateAndRender = (game_update_and_render_func *) g_GameCode.get_symbol("game_update_and_render");
    if (!g_GameUpdateAndRender) {
        fmt::print("Error: Couldn't load game_update_and_render\n");
        assert(false);
    }
}

f32 calculate_target_seconds_per_frame(HWND hWnd) {
    s32 monitorRefreshHz = 60;  // Default is 60

    HDC dc = GetDC(hWnd);
    s32 refreshRate = GetDeviceCaps(dc, VREFRESH);
    ReleaseDC(hWnd, dc);

    if (refreshRate > 1) monitorRefreshHz = refreshRate;
    return 1.0f / monitorRefreshHz;
}

// The reason we implement _main_ platform-specifically is so we can get the monitor
// refresh rate and use that as the program's target framerate and also attempt to set a granular
// sleep for when we need to wait to hit the target framerate.
//
// We can abstract these things away and have a platform-inspecific implementation
// but I don't think that provides much benefit.
s32 main() {
    game_memory gameMemory;
    gameMemory.Window = (new window::window)->init("Tetris", 1200, 600, true);

    g::dx_graphics g;
    g.init(gameMemory.Window);

    auto exePath = file::path(os_get_exe_name());

    file::path dllPath = exePath.directory();
    dllPath.combine_with("tetris.dll");

    auto dllHandle = file::handle(dllPath);

    file::path buildLockPath = exePath.directory();
    buildLockPath.combine_with("buildlock");

    auto buildLockHandle = file::handle(buildLockPath);

    f32 targetSecondsPerFrame = calculate_target_seconds_per_frame(*((HWND *) &gameMemory.Window->PlatformData));

    s64 lastCounter = os_get_time();
    s64 postFlipTime = lastCounter;

    time_t lastDllWriteTime = 0, dllCheckTimer = 0;
    while (!gameMemory.Window->Closed) {
        if (dllCheckTimer % 20 && !buildLockHandle.exists()) {
            auto writeTime = dllHandle.last_modification_time();
            if (writeTime != lastDllWriteTime) {
                reload_game_code(dllPath);
                lastDllWriteTime = writeTime;
                gameMemory.ReloadedThisFrame = true;
            } else {
                gameMemory.ReloadedThisFrame = false;
            }
        }
        ++dllCheckTimer;

        gameMemory.Window->update();

        if (g_GameUpdateAndRender) g_GameUpdateAndRender(&gameMemory, &g);

        f64 workSecondsElapsed = os_time_to_seconds(os_get_time() - lastCounter);
        // fmt::print("Target: {:10f} s, frame time: {:10f} s, frame time (including swap): {:10f}\n",
        // targetSecondsPerFrame, workSecondsElapsed, os_time_to_seconds(postFlipTime - lastCounter));
        lastCounter = os_get_time();
        g.swap();
        postFlipTime = os_get_time();
    }
}

#endif