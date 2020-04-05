#pragma once

#include "intrin/math.h"

// @TODO Replace with our own math functions in order to not depend on the runtime lib

LSTD_BEGIN_NAMESPACE

// rad / tau * 360
// deg * tau / 360
// @TODO Accurate enough?
#define TAU 6.2831853f
#define PI 3.1415926f
#define EULER 2.7182818f
#define SQRT2 1.4142135f

template <typename T>
T clamp(T value, T lower, T upper) {
    return max(lower, min(upper, value));
}

template <typename T, int Dim, bool Packed>
vec<T, Dim, Packed> clamp(const vec<T, Dim, Packed> &arg, T lower, T upper) {
    vec<T, Dim, Packed> result;
    For(range(arg.Dimension())) result(i) = clamp(arg(i), lower, upper);
    return result;
}

// ! @TODO
struct rect {
    s32 X = 0, Y = 0, Width = 0, Height = 0;

    bool operator==(rect other) const {
        return X == other.X && Y == other.Y && Width == other.Width && Height == other.Height;
    }
    bool operator!=(rect other) const { return !(*this == other); }
};

LSTD_END_NAMESPACE

#include "math/decompose_lu.h"
#include "math/decompose_qr.h"
// #include "math/decompose_svd.h"
#include "math/mat_func.h"
#include "math/quat_func.h"
#include "math/transforms/orthographic.h"
#include "math/transforms/perspective.h"
#include "math/transforms/rotation_2d.h"
#include "math/transforms/rotation_3d.h"
#include "math/transforms/scale.h"
#include "math/transforms/shear.h"
#include "math/transforms/translation.h"
#include "math/transforms/view.h"
#include "math/vec_func.h"
