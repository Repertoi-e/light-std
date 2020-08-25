#pragma once

#define VEC_DATA_2                      \
    struct {                            \
        ST x, y;                        \
    };                                  \
    ST Data[2];                         \
    swizzle<vec_data, 0, 0> xx;         \
    swizzle<vec_data, 0, 1> xy;         \
    swizzle<vec_data, 1, 0> yx;         \
    swizzle<vec_data, 1, 1> yy;         \
    swizzle<vec_data, 0, 0, 0> xxx;     \
    swizzle<vec_data, 0, 0, 1> xxy;     \
    swizzle<vec_data, 0, 1, 0> xyx;     \
    swizzle<vec_data, 0, 1, 1> xyy;     \
    swizzle<vec_data, 1, 0, 0> yxx;     \
    swizzle<vec_data, 1, 0, 1> yxy;     \
    swizzle<vec_data, 1, 1, 0> yyx;     \
    swizzle<vec_data, 1, 1, 1> yyy;     \
    swizzle<vec_data, 0, 0, 0, 0> xxxx; \
    swizzle<vec_data, 0, 0, 0, 1> xxxy; \
    swizzle<vec_data, 0, 0, 1, 0> xxyx; \
    swizzle<vec_data, 0, 0, 1, 1> xxyy; \
    swizzle<vec_data, 0, 1, 0, 0> xyxx; \
    swizzle<vec_data, 0, 1, 0, 1> xyxy; \
    swizzle<vec_data, 0, 1, 1, 0> xyyx; \
    swizzle<vec_data, 0, 1, 1, 1> xyyy; \
    swizzle<vec_data, 1, 0, 0, 0> yxxx; \
    swizzle<vec_data, 1, 0, 0, 1> yxxy; \
    swizzle<vec_data, 1, 0, 1, 0> yxyx; \
    swizzle<vec_data, 1, 0, 1, 1> yxyy; \
    swizzle<vec_data, 1, 1, 0, 0> yyxx; \
    swizzle<vec_data, 1, 1, 0, 1> yyxy; \
    swizzle<vec_data, 1, 1, 1, 0> yyyx; \
    swizzle<vec_data, 1, 1, 1, 1> yyyy;
