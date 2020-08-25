#pragma once

#define VEC_DATA_3                      \
    struct {                            \
        ST x, y, z;                     \
    };                                  \
    ST Data[3];                         \
    swizzle<vec_data, 0, 0> xx;         \
    swizzle<vec_data, 0, 1> xy;         \
    swizzle<vec_data, 0, 2> xz;         \
    swizzle<vec_data, 1, 0> yx;         \
    swizzle<vec_data, 1, 1> yy;         \
    swizzle<vec_data, 1, 2> yz;         \
    swizzle<vec_data, 2, 0> zx;         \
    swizzle<vec_data, 2, 1> zy;         \
    swizzle<vec_data, 2, 2> zz;         \
    swizzle<vec_data, 0, 0, 0> xxx;     \
    swizzle<vec_data, 0, 0, 1> xxy;     \
    swizzle<vec_data, 0, 0, 2> xxz;     \
    swizzle<vec_data, 0, 1, 0> xyx;     \
    swizzle<vec_data, 0, 1, 1> xyy;     \
    swizzle<vec_data, 0, 1, 2> xyz;     \
    swizzle<vec_data, 0, 2, 0> xzx;     \
    swizzle<vec_data, 0, 2, 1> xzy;     \
    swizzle<vec_data, 0, 2, 2> xzz;     \
    swizzle<vec_data, 1, 0, 0> yxx;     \
    swizzle<vec_data, 1, 0, 1> yxy;     \
    swizzle<vec_data, 1, 0, 2> yxz;     \
    swizzle<vec_data, 1, 1, 0> yyx;     \
    swizzle<vec_data, 1, 1, 1> yyy;     \
    swizzle<vec_data, 1, 1, 2> yyz;     \
    swizzle<vec_data, 1, 2, 0> yzx;     \
    swizzle<vec_data, 1, 2, 1> yzy;     \
    swizzle<vec_data, 1, 2, 2> yzz;     \
    swizzle<vec_data, 2, 0, 0> zxx;     \
    swizzle<vec_data, 2, 0, 1> zxy;     \
    swizzle<vec_data, 2, 0, 2> zxz;     \
    swizzle<vec_data, 2, 1, 0> zyx;     \
    swizzle<vec_data, 2, 1, 1> zyy;     \
    swizzle<vec_data, 2, 1, 2> zyz;     \
    swizzle<vec_data, 2, 2, 0> zzx;     \
    swizzle<vec_data, 2, 2, 1> zzy;     \
    swizzle<vec_data, 2, 2, 2> zzz;     \
    swizzle<vec_data, 0, 0, 0, 0> xxxx; \
    swizzle<vec_data, 0, 0, 0, 1> xxxy; \
    swizzle<vec_data, 0, 0, 0, 2> xxxz; \
    swizzle<vec_data, 0, 0, 1, 0> xxyx; \
    swizzle<vec_data, 0, 0, 1, 1> xxyy; \
    swizzle<vec_data, 0, 0, 1, 2> xxyz; \
    swizzle<vec_data, 0, 0, 2, 0> xxzx; \
    swizzle<vec_data, 0, 0, 2, 1> xxzy; \
    swizzle<vec_data, 0, 0, 2, 2> xxzz; \
    swizzle<vec_data, 0, 1, 0, 0> xyxx; \
    swizzle<vec_data, 0, 1, 0, 1> xyxy; \
    swizzle<vec_data, 0, 1, 0, 2> xyxz; \
    swizzle<vec_data, 0, 1, 1, 0> xyyx; \
    swizzle<vec_data, 0, 1, 1, 1> xyyy; \
    swizzle<vec_data, 0, 1, 1, 2> xyyz; \
    swizzle<vec_data, 0, 1, 2, 0> xyzx; \
    swizzle<vec_data, 0, 1, 2, 1> xyzy; \
    swizzle<vec_data, 0, 1, 2, 2> xyzz; \
    swizzle<vec_data, 0, 2, 0, 0> xzxx; \
    swizzle<vec_data, 0, 2, 0, 1> xzxy; \
    swizzle<vec_data, 0, 2, 0, 2> xzxz; \
    swizzle<vec_data, 0, 2, 1, 0> xzyx; \
    swizzle<vec_data, 0, 2, 1, 1> xzyy; \
    swizzle<vec_data, 0, 2, 1, 2> xzyz; \
    swizzle<vec_data, 0, 2, 2, 0> xzzx; \
    swizzle<vec_data, 0, 2, 2, 1> xzzy; \
    swizzle<vec_data, 0, 2, 2, 2> xzzz; \
    swizzle<vec_data, 1, 0, 0, 0> yxxx; \
    swizzle<vec_data, 1, 0, 0, 1> yxxy; \
    swizzle<vec_data, 1, 0, 0, 2> yxxz; \
    swizzle<vec_data, 1, 0, 1, 0> yxyx; \
    swizzle<vec_data, 1, 0, 1, 1> yxyy; \
    swizzle<vec_data, 1, 0, 1, 2> yxyz; \
    swizzle<vec_data, 1, 0, 2, 0> yxzx; \
    swizzle<vec_data, 1, 0, 2, 1> yxzy; \
    swizzle<vec_data, 1, 0, 2, 2> yxzz; \
    swizzle<vec_data, 1, 1, 0, 0> yyxx; \
    swizzle<vec_data, 1, 1, 0, 1> yyxy; \
    swizzle<vec_data, 1, 1, 0, 2> yyxz; \
    swizzle<vec_data, 1, 1, 1, 0> yyyx; \
    swizzle<vec_data, 1, 1, 1, 1> yyyy; \
    swizzle<vec_data, 1, 1, 1, 2> yyyz; \
    swizzle<vec_data, 1, 1, 2, 0> yyzx; \
    swizzle<vec_data, 1, 1, 2, 1> yyzy; \
    swizzle<vec_data, 1, 1, 2, 2> yyzz; \
    swizzle<vec_data, 1, 2, 0, 0> yzxx; \
    swizzle<vec_data, 1, 2, 0, 1> yzxy; \
    swizzle<vec_data, 1, 2, 0, 2> yzxz; \
    swizzle<vec_data, 1, 2, 1, 0> yzyx; \
    swizzle<vec_data, 1, 2, 1, 1> yzyy; \
    swizzle<vec_data, 1, 2, 1, 2> yzyz; \
    swizzle<vec_data, 1, 2, 2, 0> yzzx; \
    swizzle<vec_data, 1, 2, 2, 1> yzzy; \
    swizzle<vec_data, 1, 2, 2, 2> yzzz; \
    swizzle<vec_data, 2, 0, 0, 0> zxxx; \
    swizzle<vec_data, 2, 0, 0, 1> zxxy; \
    swizzle<vec_data, 2, 0, 0, 2> zxxz; \
    swizzle<vec_data, 2, 0, 1, 0> zxyx; \
    swizzle<vec_data, 2, 0, 1, 1> zxyy; \
    swizzle<vec_data, 2, 0, 1, 2> zxyz; \
    swizzle<vec_data, 2, 0, 2, 0> zxzx; \
    swizzle<vec_data, 2, 0, 2, 1> zxzy; \
    swizzle<vec_data, 2, 0, 2, 2> zxzz; \
    swizzle<vec_data, 2, 1, 0, 0> zyxx; \
    swizzle<vec_data, 2, 1, 0, 1> zyxy; \
    swizzle<vec_data, 2, 1, 0, 2> zyxz; \
    swizzle<vec_data, 2, 1, 1, 0> zyyx; \
    swizzle<vec_data, 2, 1, 1, 1> zyyy; \
    swizzle<vec_data, 2, 1, 1, 2> zyyz; \
    swizzle<vec_data, 2, 1, 2, 0> zyzx; \
    swizzle<vec_data, 2, 1, 2, 1> zyzy; \
    swizzle<vec_data, 2, 1, 2, 2> zyzz; \
    swizzle<vec_data, 2, 2, 0, 0> zzxx; \
    swizzle<vec_data, 2, 2, 0, 1> zzxy; \
    swizzle<vec_data, 2, 2, 0, 2> zzxz; \
    swizzle<vec_data, 2, 2, 1, 0> zzyx; \
    swizzle<vec_data, 2, 2, 1, 1> zzyy; \
    swizzle<vec_data, 2, 2, 1, 2> zzyz; \
    swizzle<vec_data, 2, 2, 2, 0> zzzx; \
    swizzle<vec_data, 2, 2, 2, 1> zzzy; \
    swizzle<vec_data, 2, 2, 2, 2> zzzz;
