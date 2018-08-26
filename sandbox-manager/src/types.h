#pragma once

#include <stdint.h>

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

#include <cstddef>

using ptr = ptrdiff_t;
using uptr = size_t;

inline size_t operator"" _KB(u64 i) {
    return (size_t) (i) << 10;
}

inline size_t operator"" _MB(u64 i) {
    return (size_t) (i) << 20;
}

inline size_t operator"" _GB(u64 i) {
    return (size_t) (i) << 30;
}

#define ArrayCount(x) ((sizeof(x)) / sizeof(x[0]))

#include <cassert>
