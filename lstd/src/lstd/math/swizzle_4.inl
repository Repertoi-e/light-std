#pragma once

#define VEC_DATA_4                      \
    struct {                            \
        ST x, y, z, w;                  \
    };                                  \
    ST Data[4];                         \
    swizzle<vec_data, 0, 0> xx;         \
    swizzle<vec_data, 0, 1> xy;         \
    swizzle<vec_data, 0, 2> xz;         \
    swizzle<vec_data, 0, 3> xw;         \
    swizzle<vec_data, 1, 0> yx;         \
    swizzle<vec_data, 1, 1> yy;         \
    swizzle<vec_data, 1, 2> yz;         \
    swizzle<vec_data, 1, 3> yw;         \
    swizzle<vec_data, 2, 0> zx;         \
    swizzle<vec_data, 2, 1> zy;         \
    swizzle<vec_data, 2, 2> zz;         \
    swizzle<vec_data, 2, 3> zw;         \
    swizzle<vec_data, 3, 0> wx;         \
    swizzle<vec_data, 3, 1> wy;         \
    swizzle<vec_data, 3, 2> wz;         \
    swizzle<vec_data, 3, 3> ww;         \
    swizzle<vec_data, 0, 0, 0> xxx;     \
    swizzle<vec_data, 0, 0, 1> xxy;     \
    swizzle<vec_data, 0, 0, 2> xxz;     \
    swizzle<vec_data, 0, 0, 3> xxw;     \
    swizzle<vec_data, 0, 1, 0> xyx;     \
    swizzle<vec_data, 0, 1, 1> xyy;     \
    swizzle<vec_data, 0, 1, 2> xyz;     \
    swizzle<vec_data, 0, 1, 3> xyw;     \
    swizzle<vec_data, 0, 2, 0> xzx;     \
    swizzle<vec_data, 0, 2, 1> xzy;     \
    swizzle<vec_data, 0, 2, 2> xzz;     \
    swizzle<vec_data, 0, 2, 3> xzw;     \
    swizzle<vec_data, 0, 3, 0> xwx;     \
    swizzle<vec_data, 0, 3, 1> xwy;     \
    swizzle<vec_data, 0, 3, 2> xwz;     \
    swizzle<vec_data, 0, 3, 3> xww;     \
    swizzle<vec_data, 1, 0, 0> yxx;     \
    swizzle<vec_data, 1, 0, 1> yxy;     \
    swizzle<vec_data, 1, 0, 2> yxz;     \
    swizzle<vec_data, 1, 0, 3> yxw;     \
    swizzle<vec_data, 1, 1, 0> yyx;     \
    swizzle<vec_data, 1, 1, 1> yyy;     \
    swizzle<vec_data, 1, 1, 2> yyz;     \
    swizzle<vec_data, 1, 1, 3> yyw;     \
    swizzle<vec_data, 1, 2, 0> yzx;     \
    swizzle<vec_data, 1, 2, 1> yzy;     \
    swizzle<vec_data, 1, 2, 2> yzz;     \
    swizzle<vec_data, 1, 2, 3> yzw;     \
    swizzle<vec_data, 1, 3, 0> ywx;     \
    swizzle<vec_data, 1, 3, 1> ywy;     \
    swizzle<vec_data, 1, 3, 2> ywz;     \
    swizzle<vec_data, 1, 3, 3> yww;     \
    swizzle<vec_data, 2, 0, 0> zxx;     \
    swizzle<vec_data, 2, 0, 1> zxy;     \
    swizzle<vec_data, 2, 0, 2> zxz;     \
    swizzle<vec_data, 2, 0, 3> zxw;     \
    swizzle<vec_data, 2, 1, 0> zyx;     \
    swizzle<vec_data, 2, 1, 1> zyy;     \
    swizzle<vec_data, 2, 1, 2> zyz;     \
    swizzle<vec_data, 2, 1, 3> zyw;     \
    swizzle<vec_data, 2, 2, 0> zzx;     \
    swizzle<vec_data, 2, 2, 1> zzy;     \
    swizzle<vec_data, 2, 2, 2> zzz;     \
    swizzle<vec_data, 2, 2, 3> zzw;     \
    swizzle<vec_data, 2, 3, 0> zwx;     \
    swizzle<vec_data, 2, 3, 1> zwy;     \
    swizzle<vec_data, 2, 3, 2> zwz;     \
    swizzle<vec_data, 2, 3, 3> zww;     \
    swizzle<vec_data, 3, 0, 0> wxx;     \
    swizzle<vec_data, 3, 0, 1> wxy;     \
    swizzle<vec_data, 3, 0, 2> wxz;     \
    swizzle<vec_data, 3, 0, 3> wxw;     \
    swizzle<vec_data, 3, 1, 0> wyx;     \
    swizzle<vec_data, 3, 1, 1> wyy;     \
    swizzle<vec_data, 3, 1, 2> wyz;     \
    swizzle<vec_data, 3, 1, 3> wyw;     \
    swizzle<vec_data, 3, 2, 0> wzx;     \
    swizzle<vec_data, 3, 2, 1> wzy;     \
    swizzle<vec_data, 3, 2, 2> wzz;     \
    swizzle<vec_data, 3, 2, 3> wzw;     \
    swizzle<vec_data, 3, 3, 0> wwx;     \
    swizzle<vec_data, 3, 3, 1> wwy;     \
    swizzle<vec_data, 3, 3, 2> wwz;     \
    swizzle<vec_data, 3, 3, 3> www;     \
    swizzle<vec_data, 0, 0, 0, 0> xxxx; \
    swizzle<vec_data, 0, 0, 0, 1> xxxy; \
    swizzle<vec_data, 0, 0, 0, 2> xxxz; \
    swizzle<vec_data, 0, 0, 0, 3> xxxw; \
    swizzle<vec_data, 0, 0, 1, 0> xxyx; \
    swizzle<vec_data, 0, 0, 1, 1> xxyy; \
    swizzle<vec_data, 0, 0, 1, 2> xxyz; \
    swizzle<vec_data, 0, 0, 1, 3> xxyw; \
    swizzle<vec_data, 0, 0, 2, 0> xxzx; \
    swizzle<vec_data, 0, 0, 2, 1> xxzy; \
    swizzle<vec_data, 0, 0, 2, 2> xxzz; \
    swizzle<vec_data, 0, 0, 2, 3> xxzw; \
    swizzle<vec_data, 0, 0, 3, 0> xxwx; \
    swizzle<vec_data, 0, 0, 3, 1> xxwy; \
    swizzle<vec_data, 0, 0, 3, 2> xxwz; \
    swizzle<vec_data, 0, 0, 3, 3> xxww; \
    swizzle<vec_data, 0, 1, 0, 0> xyxx; \
    swizzle<vec_data, 0, 1, 0, 1> xyxy; \
    swizzle<vec_data, 0, 1, 0, 2> xyxz; \
    swizzle<vec_data, 0, 1, 0, 3> xyxw; \
    swizzle<vec_data, 0, 1, 1, 0> xyyx; \
    swizzle<vec_data, 0, 1, 1, 1> xyyy; \
    swizzle<vec_data, 0, 1, 1, 2> xyyz; \
    swizzle<vec_data, 0, 1, 1, 3> xyyw; \
    swizzle<vec_data, 0, 1, 2, 0> xyzx; \
    swizzle<vec_data, 0, 1, 2, 1> xyzy; \
    swizzle<vec_data, 0, 1, 2, 2> xyzz; \
    swizzle<vec_data, 0, 1, 2, 3> xyzw; \
    swizzle<vec_data, 0, 1, 3, 0> xywx; \
    swizzle<vec_data, 0, 1, 3, 1> xywy; \
    swizzle<vec_data, 0, 1, 3, 2> xywz; \
    swizzle<vec_data, 0, 1, 3, 3> xyww; \
    swizzle<vec_data, 0, 2, 0, 0> xzxx; \
    swizzle<vec_data, 0, 2, 0, 1> xzxy; \
    swizzle<vec_data, 0, 2, 0, 2> xzxz; \
    swizzle<vec_data, 0, 2, 0, 3> xzxw; \
    swizzle<vec_data, 0, 2, 1, 0> xzyx; \
    swizzle<vec_data, 0, 2, 1, 1> xzyy; \
    swizzle<vec_data, 0, 2, 1, 2> xzyz; \
    swizzle<vec_data, 0, 2, 1, 3> xzyw; \
    swizzle<vec_data, 0, 2, 2, 0> xzzx; \
    swizzle<vec_data, 0, 2, 2, 1> xzzy; \
    swizzle<vec_data, 0, 2, 2, 2> xzzz; \
    swizzle<vec_data, 0, 2, 2, 3> xzzw; \
    swizzle<vec_data, 0, 2, 3, 0> xzwx; \
    swizzle<vec_data, 0, 2, 3, 1> xzwy; \
    swizzle<vec_data, 0, 2, 3, 2> xzwz; \
    swizzle<vec_data, 0, 2, 3, 3> xzww; \
    swizzle<vec_data, 0, 3, 0, 0> xwxx; \
    swizzle<vec_data, 0, 3, 0, 1> xwxy; \
    swizzle<vec_data, 0, 3, 0, 2> xwxz; \
    swizzle<vec_data, 0, 3, 0, 3> xwxw; \
    swizzle<vec_data, 0, 3, 1, 0> xwyx; \
    swizzle<vec_data, 0, 3, 1, 1> xwyy; \
    swizzle<vec_data, 0, 3, 1, 2> xwyz; \
    swizzle<vec_data, 0, 3, 1, 3> xwyw; \
    swizzle<vec_data, 0, 3, 2, 0> xwzx; \
    swizzle<vec_data, 0, 3, 2, 1> xwzy; \
    swizzle<vec_data, 0, 3, 2, 2> xwzz; \
    swizzle<vec_data, 0, 3, 2, 3> xwzw; \
    swizzle<vec_data, 0, 3, 3, 0> xwwx; \
    swizzle<vec_data, 0, 3, 3, 1> xwwy; \
    swizzle<vec_data, 0, 3, 3, 2> xwwz; \
    swizzle<vec_data, 0, 3, 3, 3> xwww; \
    swizzle<vec_data, 1, 0, 0, 0> yxxx; \
    swizzle<vec_data, 1, 0, 0, 1> yxxy; \
    swizzle<vec_data, 1, 0, 0, 2> yxxz; \
    swizzle<vec_data, 1, 0, 0, 3> yxxw; \
    swizzle<vec_data, 1, 0, 1, 0> yxyx; \
    swizzle<vec_data, 1, 0, 1, 1> yxyy; \
    swizzle<vec_data, 1, 0, 1, 2> yxyz; \
    swizzle<vec_data, 1, 0, 1, 3> yxyw; \
    swizzle<vec_data, 1, 0, 2, 0> yxzx; \
    swizzle<vec_data, 1, 0, 2, 1> yxzy; \
    swizzle<vec_data, 1, 0, 2, 2> yxzz; \
    swizzle<vec_data, 1, 0, 2, 3> yxzw; \
    swizzle<vec_data, 1, 0, 3, 0> yxwx; \
    swizzle<vec_data, 1, 0, 3, 1> yxwy; \
    swizzle<vec_data, 1, 0, 3, 2> yxwz; \
    swizzle<vec_data, 1, 0, 3, 3> yxww; \
    swizzle<vec_data, 1, 1, 0, 0> yyxx; \
    swizzle<vec_data, 1, 1, 0, 1> yyxy; \
    swizzle<vec_data, 1, 1, 0, 2> yyxz; \
    swizzle<vec_data, 1, 1, 0, 3> yyxw; \
    swizzle<vec_data, 1, 1, 1, 0> yyyx; \
    swizzle<vec_data, 1, 1, 1, 1> yyyy; \
    swizzle<vec_data, 1, 1, 1, 2> yyyz; \
    swizzle<vec_data, 1, 1, 1, 3> yyyw; \
    swizzle<vec_data, 1, 1, 2, 0> yyzx; \
    swizzle<vec_data, 1, 1, 2, 1> yyzy; \
    swizzle<vec_data, 1, 1, 2, 2> yyzz; \
    swizzle<vec_data, 1, 1, 2, 3> yyzw; \
    swizzle<vec_data, 1, 1, 3, 0> yywx; \
    swizzle<vec_data, 1, 1, 3, 1> yywy; \
    swizzle<vec_data, 1, 1, 3, 2> yywz; \
    swizzle<vec_data, 1, 1, 3, 3> yyww; \
    swizzle<vec_data, 1, 2, 0, 0> yzxx; \
    swizzle<vec_data, 1, 2, 0, 1> yzxy; \
    swizzle<vec_data, 1, 2, 0, 2> yzxz; \
    swizzle<vec_data, 1, 2, 0, 3> yzxw; \
    swizzle<vec_data, 1, 2, 1, 0> yzyx; \
    swizzle<vec_data, 1, 2, 1, 1> yzyy; \
    swizzle<vec_data, 1, 2, 1, 2> yzyz; \
    swizzle<vec_data, 1, 2, 1, 3> yzyw; \
    swizzle<vec_data, 1, 2, 2, 0> yzzx; \
    swizzle<vec_data, 1, 2, 2, 1> yzzy; \
    swizzle<vec_data, 1, 2, 2, 2> yzzz; \
    swizzle<vec_data, 1, 2, 2, 3> yzzw; \
    swizzle<vec_data, 1, 2, 3, 0> yzwx; \
    swizzle<vec_data, 1, 2, 3, 1> yzwy; \
    swizzle<vec_data, 1, 2, 3, 2> yzwz; \
    swizzle<vec_data, 1, 2, 3, 3> yzww; \
    swizzle<vec_data, 1, 3, 0, 0> ywxx; \
    swizzle<vec_data, 1, 3, 0, 1> ywxy; \
    swizzle<vec_data, 1, 3, 0, 2> ywxz; \
    swizzle<vec_data, 1, 3, 0, 3> ywxw; \
    swizzle<vec_data, 1, 3, 1, 0> ywyx; \
    swizzle<vec_data, 1, 3, 1, 1> ywyy; \
    swizzle<vec_data, 1, 3, 1, 2> ywyz; \
    swizzle<vec_data, 1, 3, 1, 3> ywyw; \
    swizzle<vec_data, 1, 3, 2, 0> ywzx; \
    swizzle<vec_data, 1, 3, 2, 1> ywzy; \
    swizzle<vec_data, 1, 3, 2, 2> ywzz; \
    swizzle<vec_data, 1, 3, 2, 3> ywzw; \
    swizzle<vec_data, 1, 3, 3, 0> ywwx; \
    swizzle<vec_data, 1, 3, 3, 1> ywwy; \
    swizzle<vec_data, 1, 3, 3, 2> ywwz; \
    swizzle<vec_data, 1, 3, 3, 3> ywww; \
    swizzle<vec_data, 2, 0, 0, 0> zxxx; \
    swizzle<vec_data, 2, 0, 0, 1> zxxy; \
    swizzle<vec_data, 2, 0, 0, 2> zxxz; \
    swizzle<vec_data, 2, 0, 0, 3> zxxw; \
    swizzle<vec_data, 2, 0, 1, 0> zxyx; \
    swizzle<vec_data, 2, 0, 1, 1> zxyy; \
    swizzle<vec_data, 2, 0, 1, 2> zxyz; \
    swizzle<vec_data, 2, 0, 1, 3> zxyw; \
    swizzle<vec_data, 2, 0, 2, 0> zxzx; \
    swizzle<vec_data, 2, 0, 2, 1> zxzy; \
    swizzle<vec_data, 2, 0, 2, 2> zxzz; \
    swizzle<vec_data, 2, 0, 2, 3> zxzw; \
    swizzle<vec_data, 2, 0, 3, 0> zxwx; \
    swizzle<vec_data, 2, 0, 3, 1> zxwy; \
    swizzle<vec_data, 2, 0, 3, 2> zxwz; \
    swizzle<vec_data, 2, 0, 3, 3> zxww; \
    swizzle<vec_data, 2, 1, 0, 0> zyxx; \
    swizzle<vec_data, 2, 1, 0, 1> zyxy; \
    swizzle<vec_data, 2, 1, 0, 2> zyxz; \
    swizzle<vec_data, 2, 1, 0, 3> zyxw; \
    swizzle<vec_data, 2, 1, 1, 0> zyyx; \
    swizzle<vec_data, 2, 1, 1, 1> zyyy; \
    swizzle<vec_data, 2, 1, 1, 2> zyyz; \
    swizzle<vec_data, 2, 1, 1, 3> zyyw; \
    swizzle<vec_data, 2, 1, 2, 0> zyzx; \
    swizzle<vec_data, 2, 1, 2, 1> zyzy; \
    swizzle<vec_data, 2, 1, 2, 2> zyzz; \
    swizzle<vec_data, 2, 1, 2, 3> zyzw; \
    swizzle<vec_data, 2, 1, 3, 0> zywx; \
    swizzle<vec_data, 2, 1, 3, 1> zywy; \
    swizzle<vec_data, 2, 1, 3, 2> zywz; \
    swizzle<vec_data, 2, 1, 3, 3> zyww; \
    swizzle<vec_data, 2, 2, 0, 0> zzxx; \
    swizzle<vec_data, 2, 2, 0, 1> zzxy; \
    swizzle<vec_data, 2, 2, 0, 2> zzxz; \
    swizzle<vec_data, 2, 2, 0, 3> zzxw; \
    swizzle<vec_data, 2, 2, 1, 0> zzyx; \
    swizzle<vec_data, 2, 2, 1, 1> zzyy; \
    swizzle<vec_data, 2, 2, 1, 2> zzyz; \
    swizzle<vec_data, 2, 2, 1, 3> zzyw; \
    swizzle<vec_data, 2, 2, 2, 0> zzzx; \
    swizzle<vec_data, 2, 2, 2, 1> zzzy; \
    swizzle<vec_data, 2, 2, 2, 2> zzzz; \
    swizzle<vec_data, 2, 2, 2, 3> zzzw; \
    swizzle<vec_data, 2, 2, 3, 0> zzwx; \
    swizzle<vec_data, 2, 2, 3, 1> zzwy; \
    swizzle<vec_data, 2, 2, 3, 2> zzwz; \
    swizzle<vec_data, 2, 2, 3, 3> zzww; \
    swizzle<vec_data, 2, 3, 0, 0> zwxx; \
    swizzle<vec_data, 2, 3, 0, 1> zwxy; \
    swizzle<vec_data, 2, 3, 0, 2> zwxz; \
    swizzle<vec_data, 2, 3, 0, 3> zwxw; \
    swizzle<vec_data, 2, 3, 1, 0> zwyx; \
    swizzle<vec_data, 2, 3, 1, 1> zwyy; \
    swizzle<vec_data, 2, 3, 1, 2> zwyz; \
    swizzle<vec_data, 2, 3, 1, 3> zwyw; \
    swizzle<vec_data, 2, 3, 2, 0> zwzx; \
    swizzle<vec_data, 2, 3, 2, 1> zwzy; \
    swizzle<vec_data, 2, 3, 2, 2> zwzz; \
    swizzle<vec_data, 2, 3, 2, 3> zwzw; \
    swizzle<vec_data, 2, 3, 3, 0> zwwx; \
    swizzle<vec_data, 2, 3, 3, 1> zwwy; \
    swizzle<vec_data, 2, 3, 3, 2> zwwz; \
    swizzle<vec_data, 2, 3, 3, 3> zwww; \
    swizzle<vec_data, 3, 0, 0, 0> wxxx; \
    swizzle<vec_data, 3, 0, 0, 1> wxxy; \
    swizzle<vec_data, 3, 0, 0, 2> wxxz; \
    swizzle<vec_data, 3, 0, 0, 3> wxxw; \
    swizzle<vec_data, 3, 0, 1, 0> wxyx; \
    swizzle<vec_data, 3, 0, 1, 1> wxyy; \
    swizzle<vec_data, 3, 0, 1, 2> wxyz; \
    swizzle<vec_data, 3, 0, 1, 3> wxyw; \
    swizzle<vec_data, 3, 0, 2, 0> wxzx; \
    swizzle<vec_data, 3, 0, 2, 1> wxzy; \
    swizzle<vec_data, 3, 0, 2, 2> wxzz; \
    swizzle<vec_data, 3, 0, 2, 3> wxzw; \
    swizzle<vec_data, 3, 0, 3, 0> wxwx; \
    swizzle<vec_data, 3, 0, 3, 1> wxwy; \
    swizzle<vec_data, 3, 0, 3, 2> wxwz; \
    swizzle<vec_data, 3, 0, 3, 3> wxww; \
    swizzle<vec_data, 3, 1, 0, 0> wyxx; \
    swizzle<vec_data, 3, 1, 0, 1> wyxy; \
    swizzle<vec_data, 3, 1, 0, 2> wyxz; \
    swizzle<vec_data, 3, 1, 0, 3> wyxw; \
    swizzle<vec_data, 3, 1, 1, 0> wyyx; \
    swizzle<vec_data, 3, 1, 1, 1> wyyy; \
    swizzle<vec_data, 3, 1, 1, 2> wyyz; \
    swizzle<vec_data, 3, 1, 1, 3> wyyw; \
    swizzle<vec_data, 3, 1, 2, 0> wyzx; \
    swizzle<vec_data, 3, 1, 2, 1> wyzy; \
    swizzle<vec_data, 3, 1, 2, 2> wyzz; \
    swizzle<vec_data, 3, 1, 2, 3> wyzw; \
    swizzle<vec_data, 3, 1, 3, 0> wywx; \
    swizzle<vec_data, 3, 1, 3, 1> wywy; \
    swizzle<vec_data, 3, 1, 3, 2> wywz; \
    swizzle<vec_data, 3, 1, 3, 3> wyww; \
    swizzle<vec_data, 3, 2, 0, 0> wzxx; \
    swizzle<vec_data, 3, 2, 0, 1> wzxy; \
    swizzle<vec_data, 3, 2, 0, 2> wzxz; \
    swizzle<vec_data, 3, 2, 0, 3> wzxw; \
    swizzle<vec_data, 3, 2, 1, 0> wzyx; \
    swizzle<vec_data, 3, 2, 1, 1> wzyy; \
    swizzle<vec_data, 3, 2, 1, 2> wzyz; \
    swizzle<vec_data, 3, 2, 1, 3> wzyw; \
    swizzle<vec_data, 3, 2, 2, 0> wzzx; \
    swizzle<vec_data, 3, 2, 2, 1> wzzy; \
    swizzle<vec_data, 3, 2, 2, 2> wzzz; \
    swizzle<vec_data, 3, 2, 2, 3> wzzw; \
    swizzle<vec_data, 3, 2, 3, 0> wzwx; \
    swizzle<vec_data, 3, 2, 3, 1> wzwy; \
    swizzle<vec_data, 3, 2, 3, 2> wzwz; \
    swizzle<vec_data, 3, 2, 3, 3> wzww; \
    swizzle<vec_data, 3, 3, 0, 0> wwxx; \
    swizzle<vec_data, 3, 3, 0, 1> wwxy; \
    swizzle<vec_data, 3, 3, 0, 2> wwxz; \
    swizzle<vec_data, 3, 3, 0, 3> wwxw; \
    swizzle<vec_data, 3, 3, 1, 0> wwyx; \
    swizzle<vec_data, 3, 3, 1, 1> wwyy; \
    swizzle<vec_data, 3, 3, 1, 2> wwyz; \
    swizzle<vec_data, 3, 3, 1, 3> wwyw; \
    swizzle<vec_data, 3, 3, 2, 0> wwzx; \
    swizzle<vec_data, 3, 3, 2, 1> wwzy; \
    swizzle<vec_data, 3, 3, 2, 2> wwzz; \
    swizzle<vec_data, 3, 3, 2, 3> wwzw; \
    swizzle<vec_data, 3, 3, 3, 0> wwwx; \
    swizzle<vec_data, 3, 3, 3, 1> wwwy; \
    swizzle<vec_data, 3, 3, 3, 2> wwwz; \
    swizzle<vec_data, 3, 3, 3, 3> wwww;
