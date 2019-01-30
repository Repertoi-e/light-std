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
    const u32 Modifiers, ButtonsDown;
    const s64 MouseX, MouseY;
};

struct LE_API Mouse_Entered_Event {
    Window *WindowPtr;
};

struct LE_API Mouse_Left_Event {
    Window *WindowPtr;
};

struct LE_API Mouse_Moved_Event {
    Window *WindowPtr;
    const u32 Modifiers, ButtonsDown;
    const s64 MouseX, MouseY;
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

// Convert a mouse button from it's name to code
inline u32 mouse_button_code_from_name(const string_view &name) {
    if (name == "Left") {
        return Mouse_Button_Left;
    } else if (name == "Right") {
        return Mouse_Button_Right;
    } else if (name == "Middle") {
        return Mouse_Button_Middle;
    } else if (name == "X1") {
        return Mouse_Button_X1;
    } else if (name == "X2") {
        return Mouse_Button_X2;
    }
    assert(false);
    return 0;
}

// Convert a mouse button from it's code to name
inline string_view mouse_button_name_from_code(u32 code) {
    if (code == Mouse_Button_Left) {
        return "Left";
    } else if (code == Mouse_Button_Right) {
        return "Right";
    } else if (code == Mouse_Button_Middle) {
        return "Middle";
    } else if (code == Mouse_Button_X1) {
        return "X1";
    } else if (code == Mouse_Button_X2) {
        return "X2";
    }
    assert(false);
    return "";
}

}  // namespace le