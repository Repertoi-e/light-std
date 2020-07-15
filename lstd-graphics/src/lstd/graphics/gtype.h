#pragma once

#include "lstd/types.h"

LSTD_BEGIN_NAMESPACE

enum class gtype {
    Unknown = 0,

    BOOL,

    U8,
    U16,
    U32,

    S8,
    S16,
    S32,

    F32,

    BOOL_1x1,
    BOOL_1x2,
    BOOL_1x3,
    BOOL_1x4,
    BOOL_2x1,
    BOOL_2x2,
    BOOL_2x3,
    BOOL_2x4,
    BOOL_3x1,
    BOOL_3x2,
    BOOL_3x3,
    BOOL_3x4,
    BOOL_4x1,
    BOOL_4x2,
    BOOL_4x3,
    BOOL_4x4,

    U32_1x1,
    U32_1x2,
    U32_1x3,
    U32_1x4,
    U32_2x1,
    U32_2x2,
    U32_2x3,
    U32_2x4,
    U32_3x1,
    U32_3x2,
    U32_3x3,
    U32_3x4,
    U32_4x1,
    U32_4x2,
    U32_4x3,
    U32_4x4,

    S32_1x1,
    S32_1x2,
    S32_1x3,
    S32_1x4,
    S32_2x1,
    S32_2x2,
    S32_2x3,
    S32_2x4,
    S32_3x1,
    S32_3x2,
    S32_3x3,
    S32_3x4,
    S32_4x1,
    S32_4x2,
    S32_4x3,
    S32_4x4,

    F32_1x1,
    F32_1x2,
    F32_1x3,
    F32_1x4,
    F32_2x1,
    F32_2x2,
    F32_2x3,
    F32_2x4,
    F32_3x1,
    F32_3x2,
    F32_3x3,
    F32_3x4,
    F32_4x1,
    F32_4x2,
    F32_4x3,
    F32_4x4,

    BOOL_4 = BOOL_4x1,
    U32_4 = U32_4x1,
    S32_4 = S32_4x1,
    F32_4 = F32_4x1,

    BOOL_3 = BOOL_3x1,
    U32_3 = U32_3x1,
    S32_3 = S32_3x1,
    F32_3 = F32_3x1,

    BOOL_2 = BOOL_2x1,
    U32_2 = U32_2x1,
    S32_2 = S32_2x1,
    F32_2 = F32_2x1,

    BOOL_1 = BOOL_1x1,
    U32_1 = U32_1x1,
    S32_1 = S32_1x1,
    F32_1 = F32_1x1
};

// Returns the size of the scalar type, not the whole type, e.g. returns 32 on F32_4x4
inline gtype get_scalar_gtype(gtype type) {
    u32 v = (u32) type;
    if (v >= (u32) gtype::BOOL_1x1 && v <= (u32) gtype::BOOL_4x4) return gtype::BOOL;
    if (v >= (u32) gtype::U32_1x1 && v <= (u32) gtype::U32_4x4) return gtype::U32;
    if (v >= (u32) gtype::S32_1x1 && v <= (u32) gtype::S32_4x4) return gtype::S32;
    if (v >= (u32) gtype::F32_1x1 && v <= (u32) gtype::F32_4x4) return gtype::F32;
    return type;
}

inline s64 get_size_of_base_gtype_in_bits(gtype type) {
    if (type == gtype::BOOL) return 1;
    if (type == gtype::U8) return 8;
    if (type == gtype::S8) return 8;
    if (type == gtype::U16) return 16;
    if (type == gtype::S16) return 16;
    return 32;
}

inline s64 get_count_of_gtype(gtype type) {
    switch (type) {
        case gtype::BOOL_1x2:
        case gtype::U32_1x2:
        case gtype::S32_1x2:
        case gtype::F32_1x2:
        case gtype::BOOL_2x1:
        case gtype::U32_2x1:
        case gtype::S32_2x1:
        case gtype::F32_2x1:
            return 2;
        case gtype::BOOL_1x3:
        case gtype::U32_1x3:
        case gtype::S32_1x3:
        case gtype::F32_1x3:
        case gtype::BOOL_3x1:
        case gtype::U32_3x1:
        case gtype::S32_3x1:
        case gtype::F32_3x1:
            return 3;
        case gtype::BOOL_1x4:
        case gtype::U32_1x4:
        case gtype::S32_1x4:
        case gtype::F32_1x4:
        case gtype::BOOL_4x1:
        case gtype::U32_4x1:
        case gtype::S32_4x1:
        case gtype::F32_4x1:
            return 4;
        case gtype::BOOL_2x2:
        case gtype::U32_2x2:
        case gtype::S32_2x2:
        case gtype::F32_2x2:
            return 2 * 2;
        case gtype::BOOL_2x3:
        case gtype::U32_2x3:
        case gtype::S32_2x3:
        case gtype::F32_2x3:
            return 2 * 3;
        case gtype::BOOL_2x4:
        case gtype::U32_2x4:
        case gtype::S32_2x4:
        case gtype::F32_2x4:
            return 2 * 4;
        case gtype::BOOL_3x2:
        case gtype::U32_3x2:
        case gtype::S32_3x2:
        case gtype::F32_3x2:
            return 3 * 2;
        case gtype::BOOL_3x3:
        case gtype::U32_3x3:
        case gtype::S32_3x3:
        case gtype::F32_3x3:
            return 3 * 3;
        case gtype::BOOL_3x4:
        case gtype::U32_3x4:
        case gtype::S32_3x4:
        case gtype::F32_3x4:
            return 3 * 4;
        case gtype::BOOL_4x2:
        case gtype::U32_4x2:
        case gtype::S32_4x2:
        case gtype::F32_4x2:
            return 4 * 2;
        case gtype::BOOL_4x3:
        case gtype::U32_4x3:
        case gtype::S32_4x3:
        case gtype::F32_4x3:
            return 4 * 3;
        case gtype::BOOL_4x4:
        case gtype::U32_4x4:
        case gtype::S32_4x4:
        case gtype::F32_4x4:
            return 4 * 4;
        default:
            return 1;
    }
}

enum class shader_type { None = 0, Vertex_Shader, Fragment_Shader };

LSTD_END_NAMESPACE
