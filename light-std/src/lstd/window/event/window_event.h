#pragma once

#include "../../common.h"

LSTD_BEGIN_NAMESPACE
namespace window {

struct window;

struct window_closed_event {
    window *Window;
};

struct window_resized_event {
    window *Window;
    const u32 Width, Height;
};

struct window_gained_focus_event {
    window *Window;
};

struct window_lost_focus_event {
    window *Window;
};

struct window_moved_event {
    window *Window;
    const s32 Left, Top;
};

}  // namespace window

LSTD_END_NAMESPACE
