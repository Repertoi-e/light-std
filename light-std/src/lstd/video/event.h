#pragma once

#include "../file.h"
#include "../internal/common.h"
#include "../math/vec.h"
#include "../memory/array.h"

// These values are BIT(x) because we sometimes use them as flags
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
    Mouse_Button_X2 = Mouse_Button_5,

    Mouse_Button_Last = Mouse_Button_X2
};

// Convert a mouse button from it's name to code
inline u32 mouse_button_code_from_name(const string &name) {
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
inline const string &mouse_button_name_from_code(u32 code) {
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

// Key codes in the engine correspond to the USB HID codes.
// Each constant in this file defines the location of a key and not its
// character meaning.For example, KEY_A corresponds to A on a US
// keyboard, but it corresponds to Q on a French keyboard layout.
enum : u32 {
    Key_A = 4,
    Key_B = 5,
    Key_C = 6,
    Key_D = 7,
    Key_E = 8,
    Key_F = 9,
    Key_G = 10,
    Key_H = 11,
    Key_I = 12,
    Key_J = 13,
    Key_K = 14,
    Key_L = 15,
    Key_M = 16,
    Key_N = 17,
    Key_O = 18,
    Key_P = 19,
    Key_Q = 20,
    Key_R = 21,
    Key_S = 22,
    Key_T = 23,
    Key_U = 24,
    Key_V = 25,
    Key_W = 26,
    Key_X = 27,
    Key_Y = 28,
    Key_Z = 29,
    Key_1 = 30,
    Key_2 = 31,
    Key_3 = 32,
    Key_4 = 33,
    Key_5 = 34,
    Key_6 = 35,
    Key_7 = 36,
    Key_8 = 37,
    Key_9 = 38,
    Key_0 = 39,
    Key_Enter = 40,
    Key_Escape = 41,
    Key_Backspace = 42,
    Key_Tab = 43,
    Key_Space = 44,
    Key_Minus = 45,
    Key_Equals = 46,
    Key_LeftBracket = 47,
    Key_RightBracket = 48,
    Key_Backslash = 49,
    Key_Semicolon = 51,
    Key_Quote = 52,
    Key_Grave = 53,
    Key_Comma = 54,
    Key_Period = 55,
    Key_Slash = 56,
    Key_CapsLock = 57,
    Key_F1 = 58,
    Key_F2 = 59,
    Key_F3 = 60,
    Key_F4 = 61,
    Key_F5 = 62,
    Key_F6 = 63,
    Key_F7 = 64,
    Key_F8 = 65,
    Key_F9 = 66,
    Key_F10 = 67,
    Key_F11 = 68,
    Key_F12 = 69,
    Key_PrintScreen = 70,
    Key_ScrollLock = 71,
    Key_Pause = 72,
    Key_Insert = 73,
    Key_Home = 74,
    Key_PageUp = 75,
    Key_Delete = 76,
    Key_End = 77,
    Key_PageDown = 78,
    Key_Right = 79,
    Key_Left = 80,
    Key_Down = 81,
    Key_Up = 82,
    KeyPad_NumLock = 83,
    KeyPad_Divide = 84,
    KeyPad_Multiply = 85,
    KeyPad_Subtract = 86,
    KeyPad_Add = 87,
    KeyPad_Enter = 88,
    KeyPad_1 = 89,
    KeyPad_2 = 90,
    KeyPad_3 = 91,
    KeyPad_4 = 92,
    KeyPad_5 = 93,
    KeyPad_6 = 94,
    KeyPad_7 = 95,
    KeyPad_8 = 96,
    KeyPad_9 = 97,
    KeyPad_0 = 98,
    KeyPad_Point = 99,
    Key_NonUSBackslash = 100,
    KeyPad_Equals = 103,
    Key_F13 = 104,
    Key_F14 = 105,
    Key_F15 = 106,
    Key_F16 = 107,
    Key_F17 = 108,
    Key_F18 = 109,
    Key_F19 = 110,
    Key_F20 = 111,
    Key_F21 = 112,
    Key_F22 = 113,
    Key_F23 = 114,
    Key_F24 = 115,
    Key_Help = 117,
    Key_Menu = 118,
    Key_LeftControl = 224,
    Key_LeftShift = 225,
    Key_LeftAlt = 226,
    Key_LeftGUI = 227,
    Key_RightControl = 228,
    Key_RightShift = 229,
    Key_RightAlt = 230,
    Key_RightGUI = 231,

    Key_Last = Key_RightGUI
};

// Returns 0 on error
inline u32 key_code_from_name(const string &name);
inline string key_name_from_code(u32 code);

namespace internal {
// Implemented in *_keycode.cpp platform specific files.
extern u32 g_KeycodeHidToNative[256];
extern u32 g_KeycodeNativeToHid[256];

}  // namespace internal

enum : u32 {
    Modifier_Shift = BIT(0),
    Modifier_Control = BIT(1),
    Modifier_Alt = BIT(2),
    Modifier_Super = BIT(3),
    Modifier_CapsLock = BIT(4),
    Modifier_NumLock = BIT(5)
};

struct window;

// @TODO: Specialize this for fmt::formatter
struct event {
    enum type : s32 {
        //
        // Mouse events:
        //
        Mouse_Button_Pressed = 0,   // Sets _Button_ and _DoubleClicked_
        Mouse_Button_Released = 1,  // Sets _Button_
        Mouse_Wheel_Scrolled = 2,   // Sets _ScrollX_ and _ScrollY_
        Mouse_Moved = 3,            // Sets _X_, _Y, _DX_ and _DY_
        Mouse_Entered_Window = 4,   // Doesn't set anything
        Mouse_Left_Window = 5,      // Doesn't set anything

        //
        // Keyboard events:
        //
        Keyboard_Pressed = 6,   // Sets _KeyCode_
        Keyboard_Released = 7,  // Sets _KeyCode_
        Keyboard_Repeated = 8,  // Sets _KeyCode_

        Code_Point_Typed = 9,  // Sets _CP_ to the UTF-32 encoded code point which was typed

        //
        // Window events:
        //
        Window_Closed = 10,     // Doesn't set anything
        Window_Minimized = 11,  // Sets _Minimized_ (true if the window was minimized, false if restored)
        Window_Maximized = 12,  // Sets _Maximized_ (true if the window was maximized, false if restored)
        Window_Focused = 13,    // Sets _Focused_ (true if the window gained focus, false if lost)

        Window_Moved = 14,    // Sets _X_ and _Y_
        Window_Resized = 15,  // Sets _Width_ and _Height_

        // May not map 1:1 with Window_Resized (e.g. Retina display on Mac)
        Window_Framebuffer_Resized = 16,  // Sets _Width_ and _Height_

        Window_Refreshed = 17,              // Doesn't set anything
        Window_Content_Scale_Changed = 18,  // Sets _Scale_
        Window_Files_Dropped = 19,          // Sets _Paths_ (the array contains a list of all files dropped)

        // Use this for any platform specific behaviour that can't be handled just using this library
        // This event is sent for every single platform message (including the ones handled by the events above!)
        Window_Platform_Message_Sent = 20  // Sets _Message_, _Param1_, _Param2_
    };

    window *Window = null;  // This is always set to the window which the event originated from
    type Type = (type) -1;  // This is always set to the type of event (one of the above)

    union {
        u32 Button;   // Only set on mouse button pressed/released
        u32 KeyCode;  // Only set on keyboard pressed/released/repeated events
    };

    v2 Scale = no_init;  // Only set on Window_Content_Scale_Changed

    union {
        bool DoubleClicked;  // Only set on Mouse_Button_Pressed
        struct {
            u32 ScrollX, ScrollY;  // Only set on Mouse_Wheel_Scrolled
        };

        struct {
            u32 X, Y;    // Set on Mouse_Moved and Window_Moved
            u32 DX, DY;  // Only set on Mouse_Moved
        };

        char32_t CP;  // Only set on Code_Point_Typed

        struct {
            u32 Width, Height;  // Only set on Window_Resized
        };

        bool Minimized;  // Only set on Window_Minimized
        bool Maximized;  // Only set on Window_Maximized
        bool Focused;    // Only set on Window_Focused

        struct {
            u32 Message;  // Only set on Window_Platform_Message_Sent
            u64 Param1;   // Only set on Window_Platform_Message_Sent
            s64 Param2;   // Only set on Window_Platform_Message_Sent
        };
    };

    // Gets temporarily allocated, the event doesn't own this
    array<file::path> Paths;  // Only set on Window_Files_Dropped
};
