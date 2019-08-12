#pragma once

#include "../core.h"

#include <lstd/storage/string.h>

namespace le {

struct window;

struct key_pressed_event {
    window *Window;
    u32 KeyCode, Modifiers;
    bool Repeat;
};

struct key_released_event {
    window *Window;
    u32 KeyCode, Modifiers;
};

struct key_typed_event {
    window *Window;
    char32_t CodePoint;
};

// Key codes in the engine correspond to the USB HID codes.
// Each constant in this file defines the location of a key and not its
// character meaning.For example, KEY_A corresponds to A on a US
// keyboard, but it corresponds to Q on a French keyboard layout.

enum : u32 {
    KEY_A = 4,
    KEY_B = 5,
    KEY_C = 6,
    KEY_D = 7,
    KEY_E = 8,
    KEY_F = 9,
    KEY_G = 10,
    KEY_H = 11,
    KEY_I = 12,
    KEY_J = 13,
    KEY_K = 14,
    KEY_L = 15,
    KEY_M = 16,
    KEY_N = 17,
    KEY_O = 18,
    KEY_P = 19,
    KEY_Q = 20,
    KEY_R = 21,
    KEY_S = 22,
    KEY_T = 23,
    KEY_U = 24,
    KEY_V = 25,
    KEY_W = 26,
    KEY_X = 27,
    KEY_Y = 28,
    KEY_Z = 29,
    KEY_1 = 30,
    KEY_2 = 31,
    KEY_3 = 32,
    KEY_4 = 33,
    KEY_5 = 34,
    KEY_6 = 35,
    KEY_7 = 36,
    KEY_8 = 37,
    KEY_9 = 38,
    KEY_0 = 39,
    KEY_Enter = 40,
    KEY_Escape = 41,
    KEY_Delete = 42,
    KEY_Tab = 43,
    KEY_Space = 44,
    KEY_Minus = 45,
    KEY_Equals = 46,
    KEY_LeftBracket = 47,
    KEY_RightBracket = 48,
    KEY_Backslash = 49,
    KEY_Semicolon = 51,
    KEY_Quote = 52,
    KEY_Grave = 53,
    KEY_Comma = 54,
    KEY_Period = 55,
    KEY_Slash = 56,
    KEY_CapsLock = 57,
    KEY_F1 = 58,
    KEY_F2 = 59,
    KEY_F3 = 60,
    KEY_F4 = 61,
    KEY_F5 = 62,
    KEY_F6 = 63,
    KEY_F7 = 64,
    KEY_F8 = 65,
    KEY_F9 = 66,
    KEY_F10 = 67,
    KEY_F11 = 68,
    KEY_F12 = 69,
    KEY_PrintScreen = 70,
    KEY_ScrollLock = 71,
    KEY_Pause = 72,
    KEY_Insert = 73,
    KEY_Home = 74,
    KEY_PageUp = 75,
    KEY_DeleteForward = 76,
    KEY_End = 77,
    KEY_PageDown = 78,
    KEY_Right = 79,
    KEY_Left = 80,
    KEY_Down = 81,
    KEY_Up = 82,
    KP_NumLock = 83,
    KP_Divide = 84,
    KP_Multiply = 85,
    KP_Subtract = 86,
    KP_Add = 87,
    KP_Enter = 88,
    KP_1 = 89,
    KP_2 = 90,
    KP_3 = 91,
    KP_4 = 92,
    KP_5 = 93,
    KP_6 = 94,
    KP_7 = 95,
    KP_8 = 96,
    KP_9 = 97,
    KP_0 = 98,
    KP_Point = 99,
    KEY_NonUSBackslash = 100,
    KP_Equals = 103,
    KEY_F13 = 104,
    KEY_F14 = 105,
    KEY_F15 = 106,
    KEY_F16 = 107,
    KEY_F17 = 108,
    KEY_F18 = 109,
    KEY_F19 = 110,
    KEY_F20 = 111,
    KEY_F21 = 112,
    KEY_F22 = 113,
    KEY_F23 = 114,
    KEY_F24 = 115,
    KEY_Help = 117,
    KEY_Menu = 118,
    KEY_LeftControl = 224,
    KEY_LeftShift = 225,
    KEY_LeftAlt = 226,
    KEY_LeftGUI = 227,
    KEY_RightControl = 228,
    KEY_RightShift = 229,
    KEY_RightAlt = 230,
    KEY_RightGUI = 231
};

// This following code has been automatically generated:

inline const char KEYID_NAME[598] =
    "0\0001\0002\0003\0004\0005\0006\0007\0008\0009\0A\0B\0Backslash\0C\0CapsLoc"
    "k\0Comma\0D\0Delete\0DeleteForward\0Down\0E\0End\0Enter\0Equals\0Escape\0F"
    "\0F1\0F10\0F11\0F12\0F13\0F14\0F15\0F16\0F17\0F18\0F19\0F2\0F20\0F21\0F22\0"
    "F23\0F24\0F3\0F4\0F5\0F6\0F7\0F8\0F9\0G\0Grave\0H\0Help\0Home\0I\0Insert\0J"
    "\0K\0KP0\0KP1\0KP2\0KP3\0KP4\0KP5\0KP6\0KP7\0KP8\0KP9\0KPAdd\0KPDivide\0KPE"
    "nter\0KPEquals\0KPMultiply\0KPNumLock\0KPPoint\0KPSubtract\0L\0Left\0LeftAl"
    "t\0LeftBracket\0LeftControl\0LeftGUI\0LeftShift\0M\0Menu\0Minus\0N\0NonUSBa"
    "ckslash\0O\0P\0PageDown\0PageUp\0Pause\0Period\0PrintScreen\0Q\0Quote\0R\0R"
    "ight\0RightAlt\0RightBracket\0RightControl\0RightGUI\0RightShift\0S\0Scroll"
    "Lock\0Semicolon\0Slash\0Space\0T\0Tab\0U\0Up\0V\0W\0X\0Y\0Z";

inline s16 g_KeyidOff[256] = {
    -1,  -1,  -1,  -1,  20,  22,  34,  51,  79,  105, 194, 202, 214, 223, 225, 339, 396, 409, 426, 428, 471, 479,
    542, 577, 583, 588, 590, 592, 594, 596, 2,   4,   6,   8,   10,  12,  14,  16,  18,  0,   85,  98,  53,  579,
    571, 403, 91,  354, 496, 24,  -1,  555, 473, 196, 45,  452, 565, 36,  107, 150, 173, 176, 179, 182, 185, 188,
    191, 110, 114, 118, 459, 544, 446, 216, 209, 439, 60,  81,  430, 481, 341, 74,  585, 310, 273, 299, 328, 267,
    282, 231, 235, 239, 243, 247, 251, 255, 259, 263, 227, 320, 411, -1,  -1,  290, 122, 126, 130, 134, 138, 142,
    146, 153, 157, 161, 165, 169, -1,  204, 398, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  366, 386, 346, 378, 509, 531, 487, 522, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1};

inline s32 g_KeyidOrder[119] = {
    39,  30, 31, 32, 33,  34,  35,  36,  37,  38,  4,   5,   49,  6,   57,  54,  7,   42,  76,  81, 8,  77, 40, 46,
    41,  9,  58, 67, 68,  69,  104, 105, 106, 107, 108, 109, 110, 59,  111, 112, 113, 114, 115, 60, 61, 62, 63, 64,
    65,  66, 10, 53, 11,  117, 74,  12,  73,  13,  14,  98,  89,  90,  91,  92,  93,  94,  95,  96, 97, 87, 84, 88,
    103, 85, 83, 99, 86,  15,  80,  226, 47,  224, 227, 225, 16,  118, 45,  17,  100, 18,  19,  78, 75, 72, 55, 70,
    20,  52, 21, 79, 230, 48,  228, 231, 229, 22,  71,  51,  56,  44,  23,  43,  24,  82,  25,  26, 27, 28, 29};

inline u32 key_code_from_name(const string &name) {
    u32 l = 0, r = 119;
    while (l < r) {
        auto m = (l + r) / 2;
        auto x = g_KeyidOrder[m];
        auto c = name.compare(string(KEYID_NAME + g_KeyidOff[x]));
        if (c < 0) {
            r = m;
        } else if (c > 0) {
            l = m + 1;
        } else {
            return x;
        }
    }
    return -1;
}

inline string key_name_from_code(u32 code) {
    if (code < 0 || code > 255) return "";
    s32 off = g_KeyidOff[code];
    if (off == (u16) -1) return "";
    return string(KEYID_NAME + off);
}

// Implemented in *_keycode.cpp platform specific files.
extern u32 g_KeycodeHidToNative[256];
extern u32 g_KeycodeNativeToHid[256];

}  // namespace le