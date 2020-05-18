#pragma once

#include "../math.h"
#include "../memory/pixel_buffer.h"

struct HICON__;

LSTD_BEGIN_NAMESPACE

enum os_cursor : u32 {
    OS_APPSTARTING,  // Standard arrow and small hourglass

    OS_ARROW,
    OS_IBEAM,
    OS_CROSSHAIR,
    OS_HAND,

    OS_HELP,  // Arrow and question mark
    OS_NO,    // Slashed circle

    OS_RESIZE_ALL,
    OS_RESIZE_NESW,
    OS_RESIZE_NS,
    OS_RESIZE_NWSE,
    OS_RESIZE_WE,

    OS_UP_ARROW,  // Vertical arrow
    OS_WAIT       // Hourglass
};

struct cursor {
    union platform_data {
        struct {
            HICON__ *hCursor = null;
            bool ShouldDestroy = false;
        } Win32;
    } PlatformData{};

    cursor() = default;
    cursor(const pixel_buffer &image, vec2<s32> hot);
    cursor(os_cursor osCursor);

    void release();

    // We keep track of created cursors in a linked list
    cursor *Next = null;
};

LSTD_END_NAMESPACE
