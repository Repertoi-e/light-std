#pragma once

#include "../core.hpp"

namespace le {

struct LE_API Window;

struct LE_API Window_Closed_Event {
    Window *WindowPtr;
};

struct LE_API Window_Resized_Event {
    Window *WindowPtr;
    const u32 Width, Height;
};

struct LE_API Window_Gained_Focus_Event {
    Window *WindowPtr;
};

struct LE_API Window_Lost_Focus_Event {
    Window *WindowPtr;
};

struct LE_API Window_Moved_Event {
    Window *WindowPtr;
    const s32 Left, Top;
};

}  // namespace le