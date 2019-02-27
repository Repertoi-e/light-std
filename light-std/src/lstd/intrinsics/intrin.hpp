#pragma once

#include "../common.hpp"

LSTD_BEGIN_NAMESPACE

union IEEEf2bits {
    f32 F;
    struct {
#if ENDIAN == LITTLE_ENDIAN
        u32 Sign : 1;
        u32 Exp : 8;
        u32 Man : 23;
#else  // _BIG_ENDIAN
        u32 Man : 23;
        u32 Exp : 8;
        u32 Sign : 1;
#endif
    } Bits;
};

union IEEEd2bits {
    f64 D;
    struct {
#if ENDIAN == LITTLE_ENDIAN
        u32 Manl : 32;
        u32 Manh : 20;
        u32 Exp : 11;
        u32 Sign : 1;
#else  // _BIG_ENDIAN
        u32 Sign : 1;
        u32 Exp : 11;
        u32 Manh : 20;
        u32 Manl : 32;
#endif
    } Bits;
};

constexpr byte sign_bit(f32 value) { return IEEEf2bits{value}.Bits.Sign; }
constexpr byte sign_bit(f64 value) { return IEEEd2bits{value}.Bits.Sign; }

inline s32 absolute_value(s32 x) {
    if (x < 0) x = -x;
    return x;
}

inline s64 absolute_value(s64 x) {
    if (x < 0) x = -x;
    return x;
}

inline f32 absolute_value(f32 x) {
    if (x < 0) x = -x;
    return x;
}

inline f64 absolute_value(f64 x) {
    if (x < 0) x = -x;
    return x;
}

LSTD_END_NAMESPACE