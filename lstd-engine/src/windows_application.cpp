#include "le/core.hpp"

#if OS == WINDOWS

#include "le/application/application.hpp"

#include <lstd/fmt.hpp>

#undef MAC
#undef _MAC
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace le {

application *application::s_Instance = null;

// The reason we implement Application::run platform-specifically is so we can get the monitor
// refresh rate and use that as the program's target framerate and also attempt to set a granular
// sleep for when we need to wait to hit the target framerate.
//
// We can abstract these things away and have a platform-inspecific implementation but I don't
// think that provides much benefit.
void application::run() {
    HWND hWnd = (HWND) Window->PlatformData;

    // It's always fun to use undocumented kernel functions :eyes:
    s32(__stdcall * NtDelayExecutionFunc)(BOOL, PLARGE_INTEGER) =
        (s32(*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");
    s32(__stdcall * ZwSetTimerResolutionFunc)(ULONG, BOOLEAN, PULONG) =
        (s32(*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");

    ULONG ignored;
    ZwSetTimerResolutionFunc(1, true, &ignored);

    s32 monitorRefreshHz = 60;

    HDC dc = GetDC(hWnd);
    int Win32RefreshRate = GetDeviceCaps(dc, VREFRESH);
    ReleaseDC(hWnd, dc);

    if (Win32RefreshRate > 1) {
        monitorRefreshHz = Win32RefreshRate;
    }

    f32 gameUpdateHz = (f32) monitorRefreshHz;
    f32 targetSecondsPerFrame = (1.0f / (gameUpdateHz));

    s64 lastCounter = os_get_wallclock();
    s64 flipWallClock;

    while (!Window->Closed) {
        Window->update();

        For(_Layers) { it->on_update(targetSecondsPerFrame); }

        f64 workSecondsElapsed = os_get_elapsed_in_seconds(lastCounter, os_get_wallclock());
        f64 compensate = workSecondsElapsed;
        u32 actualMs, whiles = 0;

        if (compensate < targetSecondsPerFrame) {
            s64 before = os_get_wallclock();
            auto ms = (u32)(1000.0f * (targetSecondsPerFrame - compensate));
            if (ms > 3) {
                ms -= 3;

                LARGE_INTEGER interval;
                interval.QuadPart = -1 * (s32)(ms * 10000.0f);
                NtDelayExecutionFunc(false, &interval);
            }
            s64 now = os_get_wallclock();
            actualMs = (u32)(1000.0f * os_get_elapsed_in_seconds(before, now));

            if (os_get_elapsed_in_seconds(lastCounter, now) > targetSecondsPerFrame) {
                fmt::print("(windows_application.cpp): Slept for too long! (Didn't hit target framerate)\n");
            }

            while (compensate < targetSecondsPerFrame) {
                ++whiles;
                compensate = os_get_elapsed_in_seconds(lastCounter, os_get_wallclock());
            }
        } else {
            fmt::print("(windows_application.cpp): Frame took too long! (Didn't hit target framerate)\n");
        }

        fmt::print("(windows_application.cpp): Target: {} s, work done: {} s, slept: {} ms, {} whiles\n",
                   targetSecondsPerFrame, workSecondsElapsed, actualMs, whiles);

        s64 endCounter = os_get_wallclock();
        lastCounter = endCounter;

        // TODO: Swap buffers here!

        // At the moment flipWallClock is not used for anything,
        // but will be useful when we do audio
        flipWallClock = os_get_wallclock();
    }

    os_exit_program(0);
}
}  // namespace le

#endif