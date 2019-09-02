#pragma once

#include "../../storage/string.h"

LSTD_BEGIN_NAMESPACE

struct window;

enum : char { Key_Released = 0, Key_Pressed = 1, Key_Repeated = 2 };

struct key_event {
    window *Window;
    u32 KeyCode, Modifiers;
    char Action;
};

struct code_point_typed_event {
    window *Window;
    char32_t CP;
};

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

inline u32 key_code_from_name(string name);
inline string key_name_from_code(u32 code);

namespace internal {
// Implemented in *_keycode.cpp platform specific files.
extern u32 g_KeycodeHidToNative[256];
extern u32 g_KeycodeNativeToHid[256];

}  // namespace internal

LSTD_END_NAMESPACE
