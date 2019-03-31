#pragma once

#include "../core.hpp"

namespace le {

struct LE_API window;

struct LE_API window_closed_event {
    window *Window;
};

struct LE_API window_resized_event {
    window *Window;
    const u32 Width, Height;
};

struct LE_API window_gained_focus_event {
    window *Window;
};

struct LE_API window_lost_focus_event {
    window *Window;
};

struct LE_API window_moved_event {
    window *Window;
    const s32 Left, Top;
};

}  // namespace le