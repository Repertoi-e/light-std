#pragma once

#include "../core.hpp"

namespace le {

struct LE_API Window;

struct LE_API Mouse_Button_Pressed_Event {
    Window *WindowPtr;
    const u32 Button, Modifiers;
    const s64 MouseX, MouseY;
};

struct LE_API Mouse_Button_Released_Event {
    Window *WindowPtr;
    const u32 Button, Modifiers;
    const s64 MouseX, MouseY;
};

struct LE_API Mouse_Scrolled_Event {
    Window *WindowPtr;
    const s64 DeltaX, DeltaY;
    const s64 MouseX, MouseY;
    const u32 Modifiers, ButtonsDown;
};

struct LE_API Mouse_Entered_Event {
    Window *WindowPtr;
};

struct LE_API Mouse_Left_Event {
    Window *WindowPtr;
};

struct LE_API Mouse_Moved_Event {
    Window *WindowPtr;
    const s64 MouseX, MouseY;
    const u32 Modifiers, ButtonsDown;
};

// These values are BIT(x) because we sometimes use them as flags (e.g. Mouse_Moved_Event)

enum : u32 {
    Mouse_Button_1 = BIT(0),
    Mouse_Button_2 = BIT(1),
    Mouse_Button_3 = BIT(2),
    Mouse_Button_4 = BIT(3),
    Mouse_Button_5 = BIT(4),
    Mouse_Button_Left = Mouse_Button_1,
    Mouse_Button_Right = Mouse_Button_2,
    Mouse_Button_Middle = Mouse_Button_3,
    Mouse_Button_X1 = Mouse_Button_4,
    Mouse_Button_X2 = Mouse_Button_5
};

}  // namespace le