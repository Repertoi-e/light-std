#pragma once

#define VEC_DATA_1                  \
    struct {                        \
        ST x;                       \
    };                              \
    ST Data[1];                     \
    swizzle<vec_data, 0, 0> xx;     \
    swizzle<vec_data, 0, 0, 0> xxx; \
    swizzle<vec_data, 0, 0, 0, 0> xxxx;
