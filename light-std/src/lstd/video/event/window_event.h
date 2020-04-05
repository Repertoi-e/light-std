#pragma once

#include "../../file.h"
#include "../../storage/array.h"
#include "../../math.h"

LSTD_BEGIN_NAMESPACE

struct window;

struct window_closed_event {
    window *Window;
};

struct window_resized_event {
    window *Window;
    s32 Width, Height;
};

// May not map 1:1 with window size (e.g. Retina display on Mac)
struct window_framebuffer_resized_event {
    window *Window;
    s32 Width, Height;
};

struct window_focused_event {
    window *Window;
    bool Focused;
};

struct window_minimized_event {
    window *Window;
    bool Minimized;
};

struct window_maximized_event {
    window *Window;
    bool Maximized;
};

struct window_moved_event {
    window *Window;
    s32 X, Y;
};

struct window_refreshed_event {
    window *Window;
};

struct window_content_scale_changed_event {
    window *Window;
    v2 Scale;
};

struct window_files_dropped_event {
    window *Window;
    array<file::path> Paths;  // Gets temporarily allocated, the event doesn't own this.
};

inline window_files_dropped_event *clone(window_files_dropped_event *dest, window_files_dropped_event src) {
    dest->Window = src.Window;
    clone(&dest->Paths, src.Paths);
	return dest;
}

struct window_generic_platform_message_event {
    window *Window;
    u32 Message;
    uptr_t Param1;
    ptr_t Param2;
};

LSTD_END_NAMESPACE
