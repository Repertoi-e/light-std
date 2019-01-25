#pragma once

#include "../common.hpp"

CPPU_BEGIN_NAMESPACE

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

CPPU_END_NAMESPACE