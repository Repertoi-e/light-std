#pragma once

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"

#include "math/mat4.h"

#include "math/quat.h"

LSTD_BEGIN_NAMESPACE

struct rect {
    s32 X = 0, Y = 0, Width = 0, Height = 0;

    bool operator==(rect other) const {
        return X == other.X && Y == other.Y && Width == other.Width && Height == other.Height;
    }
    bool operator!=(rect other) const { return !(*this == other); }
};

using vec2i8 = tvec2<s8>;
using vec2i16 = tvec2<s16>;
using vec2i = tvec2<s32>;
using vec2i64 = tvec2<s64>;

using vec2ui8 = tvec2<u8>;
using vec2ui16 = tvec2<u16>;
using vec2ui = tvec2<u32>;
using vec2ui64 = tvec2<u64>;

using vec3i8 = tvec3<s8>;
using vec3i16 = tvec3<s16>;
using vec3i = tvec3<s32>;
using vec3i64 = tvec3<s64>;

using vec3ui8 = tvec3<u8>;
using vec3ui16 = tvec3<u16>;
using vec3ui = tvec3<u32>;
using vec3ui64 = tvec3<u64>;

using vec4i8 = tvec4<s8>;
using vec4i16 = tvec4<s16>;
using vec4i = tvec4<s32>;
using vec4i64 = tvec4<s64>;

using vec4ui8 = tvec4<u8>;
using vec4ui16 = tvec4<u16>;
using vec4ui = tvec4<u32>;
using vec4ui64 = tvec4<u64>;

LSTD_END_NAMESPACE
