#pragma once

#include "../types.h"

union ieee754_f32 {
    f32 F;
    u32 U;

    // This is the IEEE 754 single-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 8;
        u32 M : 23;
#else
        u32 M : 23;
        u32 E : 8;
        u32 S : 1;
#endif
    } ieee;

    // This format makes it easier to see if a NaN is a signalling NaN.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 8;
        u32 N : 1;
        u32 M : 22;
#else
        u32 M : 22;
        u32 N : 1;
        u32 E : 8;
        u32 S : 1;
#endif
    } ieee_nan;
};

union ieee754_f64 {
    f64 F;
    u64 U;

    // This is the IEEE 754 single-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 11;
        u32 M0 : 20;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 20;
        u32 E : 11;
        u32 S : 1;
#endif
    } ieee;

    // This format makes it easier to see if a NaN is a signalling NaN.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 11;
        u32 N : 1;
        u32 M0 : 19;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 19;
        u32 N : 1;
        u32 E : 11;
        u32 S : 1;
#endif
    } ieee_nan;
};

// @Wrong
// This seems wrong, not sure.
// sizeof(ieee854_lf64) is 16 but sizeof(long double) in MSVC is 8
union ieee854_lf64 {
    lf64 F;
    u64 U;

    // This is the IEEE 854 double-extended-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 15;
        u32 Empty : 16;
        u32 M0 : 32;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 32;
        u32 E : 15;
        u32 S : 1;
        u32 Empty : 16;
#endif
    } ieee;

    // This is for NaNs in the IEEE 854 double-extended-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 15;
        u32 Empty : 16;
        u32 One : 1;
        u32 N : 1;
        u32 M0 : 30;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 30;
        u32 N : 1;
        u32 One : 1;
        u32 E : 15;
        u32 S : 1;
        u32 Empty : 16;
#endif
    } ieee_nan;
};