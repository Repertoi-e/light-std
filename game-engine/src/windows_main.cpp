#include "le/core.h"

#if defined LE_BUILDING_GAME
#error Error
#endif

#if OS == WINDOWS

#include "le/game.h"

#include <lstd/file.h>
#include <lstd/io/fmt.h>
#include <lstd/memory/dynamic_library.h>
#include <lstd/os.h>

#undef MAC
#undef _MAC
#include <Windows.h>
#define VREFRESH 116

using namespace le;

static HWND g_HWND;
static dynamic_library g_GameCode;
static game_update_and_render_func *g_GameUpdateAndRender = null;

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

// It's always fun to use undocumented kernel functions :eyes:
s32(__stdcall *NtDelayExecutionFunc)(BOOL, PLARGE_INTEGER) = (s32(*)(BOOL, PLARGE_INTEGER))
    GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");
s32(__stdcall *ZwSetTimerResolutionFunc)(ULONG, BOOLEAN, PULONG) = (s32(*)(ULONG, BOOLEAN, PULONG))
    GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");

f32 calculate_target_seconds_per_frame() {
    s32 monitorRefreshHz = 60;  // Default is 60

    HDC dc = GetDC(g_HWND);
    s32 refreshRate = GetDeviceCaps(dc, VREFRESH);
    ReleaseDC(g_HWND, dc);

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
    gameMemory.Window = (new window)->init("Tetris", 1200, 600);
    
	g_HWND = (HWND) gameMemory.Window->PlatformData;

    auto exePath = file::path(os_get_exe_name());

    file::path dllPath = exePath.directory();
    dllPath.combine_with("tetris.dll");

    auto dllHandle = file::handle(dllPath);

    file::path buildLockPath = exePath.directory();
    buildLockPath.combine_with("buildlock");

    auto buildLockHandle = file::handle(buildLockPath);

    f32 targetSecondsPerFrame = calculate_target_seconds_per_frame();

    s64 lastCounter = os_get_time();
    s64 flipWallClock;

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

        if (g_GameUpdateAndRender) g_GameUpdateAndRender(&gameMemory);

        f64 workSecondsElapsed = os_time_to_seconds(os_get_time() - lastCounter);
        f64 compensate = workSecondsElapsed;
        u32 actualMs, whiles = 0;

        if (compensate < targetSecondsPerFrame) {
            s64 before = os_get_time();
            auto ms = (u64)(1000.0f * (targetSecondsPerFrame - compensate));

            // Check for at least 3 ms or we seem to oversleep otherwise
            if (ms > 3) {
                ms -= 3;

                LARGE_INTEGER interval;
                interval.QuadPart = -(s64)(ms * 10000);
                NtDelayExecutionFunc(false, &interval);
            }
            s64 now = os_get_time();
            actualMs = (u32)(1000.0f * os_time_to_seconds(now - before));

            if (os_time_to_seconds(now - lastCounter) > targetSecondsPerFrame) {
                fmt::print("(windows_main.cpp): Slept for too long! (Didn't hit target framerate)\n");
            }

            while (compensate < targetSecondsPerFrame) {
                ++whiles;
                compensate = os_time_to_seconds(os_get_time() - lastCounter);
            }
        } else {
            fmt::print("(windows_main.cpp): Frame took too long! (Didn't hit target framerate)\n");
        }

        // fmt::print("(windows_main.cpp): Target: {:10f} s, work done: {:10f} s, slept: {:4} ms, {:10} whiles\n",
        //          targetSecondsPerFrame, workSecondsElapsed, actualMs, whiles);

        s64 endCounter = os_get_time();
        lastCounter = endCounter;

        // @TODO: Swap buffers here!

        // At the moment flipWallClock is not used for anything,
        // but will be useful when we do audio
        flipWallClock = os_get_time();
    }
    os_exit(0);
}

#endif