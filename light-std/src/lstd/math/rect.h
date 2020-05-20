#pragma once

#include "../types.h"

struct rect {
    s32 Top = 0, Left = 0;
    s32 Bot = 0, Right = 0;

    s32 width() { return Right - Left; }
    s32 height() { return Bot - Top; }

    bool operator==(rect other) const {
        return Top == other.Top && Left == other.Left && Bot == other.Bot && Right == other.Right;
    }
    bool operator!=(rect other) const { return !(*this == other); }
};