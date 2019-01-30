#include "le/core.hpp"

#if OS == WINDOWS

#include "le/application/application.hpp"

#include <lstd/fmt.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <timeapi.h>

namespace le {

Application *Application::s_Instance = null;

// The reason we implement Application::run platform-specifically is so we can get the monitor
// refresh rate and use that as the program's target framerate and also attempt to set a granular
// sleep for when we need to wait to hit the target framerate.
//
// We can abstract these things away and have a platform-inspecific implementation but I don't
// think that provides much benefit.
void Application::run() {
    HWND hWnd = (HWND) WindowPtr->PlatformData;

    // Request 1 ms granularity for Sleep
    bool sleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);

    s32 monitorRefreshHz = 60;

    HDC dc = GetDC(hWnd);
    int Win32RefreshRate = GetDeviceCaps(dc, VREFRESH);
    ReleaseDC(hWnd, dc);

    if (Win32RefreshRate > 1) {
        monitorRefreshHz = Win32RefreshRate;
    }

    f32 gameUpdateHz = ((f32) monitorRefreshHz / 2.0f);
    f32 targetSecondsPerFrame = (1.0f / (gameUpdateHz));

    s64 lastCounter = os_get_wallclock();
    s64 flipWallClock = lastCounter;

    while (!WindowPtr->Closed) {
        WindowPtr->update();

        For(Layers) { it->on_update(targetSecondsPerFrame); }

        f64 workSecondsElapsed = os_get_elapsed_in_seconds(lastCounter, os_get_wallclock());

        if (workSecondsElapsed < targetSecondsPerFrame) {
            if (sleepIsGranular) {
                auto ms = (u32)(1000.0f * (targetSecondsPerFrame - workSecondsElapsed));
                // Sometimes 2 ms is enough to cause error (at least on my machine), so be safe
                // and sleep for 1 ms shorter than we need (the rest is compensated by the spin lock)
                if (ms > 2) {
                    Sleep(ms - 2);
                }
            }
            if (targetSecondsPerFrame < os_get_elapsed_in_seconds(lastCounter, os_get_wallclock())) {
                fmt::print("(windows_application.cpp): Slept for too long! (Didn't hit target framerate)\n");
            }

            while (workSecondsElapsed < targetSecondsPerFrame) {
                workSecondsElapsed = os_get_elapsed_in_seconds(lastCounter, os_get_wallclock());
            }
        } else {
            fmt::print("(windows_application.cpp): Frame took too long! (Didn't hit target framerate)\n");
        }

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