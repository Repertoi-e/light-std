#pragma once

#include "identity.h"

LSTD_BEGIN_NAMESPACE

template <typename T, s64 Dim, bool Packed>
struct perspective_helper : non_copyable {
    vec<T, Dim - 1, Packed> Ratios;
    T FovX;
    T NearPlane, FarPlane;
    T ProjNearPlane = 0, ProjFarPlane = 1;

    perspective_helper(T fovX, const vec<T, Dim - 1, Packed> &ratios, T nearPlane, T farPlane, T projNearPlane = 0,
                       T projFarPlane = 1)
        : FovX(fovX),
          Ratios(ratios),
          NearPlane(nearPlane),
          FarPlane(farPlane),
          ProjNearPlane(projNearPlane),
          ProjFarPlane(projFarPlane) {}

    template <typename U, bool MPacked>
    operator mat<U, Dim + 1, Dim + 1, MPacked>() const {
        using UVec = vec<U, Dim - 1, Packed>;

        mat<U, Dim + 1, Dim + 1, MPacked> m = zero();
        assert((NearPlane < 0 && FarPlane < NearPlane) || (0 < NearPlane && NearPlane < FarPlane));

        // Layout be like (precede_vector):
        // w 0 0 0
        // 0 h 0 0
        // 0 0 A B
        // 0 0 C 0
        auto absFovX = abs(U(FovX));
        U N = U(NearPlane);
        U F = U(FarPlane);
        U n = U(ProjNearPlane);
        U f = U(ProjFarPlane);
        U C = U(NearPlane) < U(0) ? U(-1) : U(1);
        U A = C * (f * F - n * N) / (F - N);
        U B = C * F * N * (n - f) / (F - N);

        UVec adjRatios = U(Ratios[0]) / UVec(Ratios);
        U w = (U) tan(U(0.5) * absFovX);
        adjRatios /= w;
        For(range(adjRatios.Dim)) m(it, it) = adjRatios[it];

        m(-2, -2) = A;
        m(-1, -2) = B;
        m(-2, -1) = C;
        return m;
    }
};

// Creates a general, n-dimensional perspective projection matrix.
// Field of view on the first axis (usually denoted X) in radians.
// Aspect ratio (or ratios in higher dimensions). FovX/FovY.
// Near bound of the projected volume on the last axis (Z in 3D).
// Far bound of the projected volume on the last axis (Z in 3D).
// The near plane is taken here after projection.
// The far plane is taken here after projection.
// The pre-projection near and far planes must be on the same side of the last axis,
//   i.e. both negative or both positive.
// The post-projection near and far planes can be arbitrary, that is, near larger, near smaller,
//   both positive, both negative, one positive the other negative, etc.
// Negative _ratios_ invert image on the axis which has a negative value.
template <typename T, s64 DimMinus1, bool Packed>
auto perspective(T fovX, const vec<T, DimMinus1, Packed> &ratios, T nearPlane, T farPlane, T projNearPlane,
                 T projFarPlane) {
    using NonIntegral = type_select_t<is_integral_v<T>, f32, T>;
    return perspective_helper<NonIntegral, DimMinus1 + 1, Packed>{fovX,     ratios,        nearPlane,
                                                                  farPlane, projNearPlane, projFarPlane};
}

// Creates a 2D projection matrix.
// Field of view.
// Lower bound of the volume on the Y axis.
// Upper bound of the volume on the Y axis.
// Near plane is taken here after projection.
// Far plane is taken here after projection.
// The pre-projection near and far planes must be on the same side of the last axis,
//   i.e. both negative or both positive.
// The post-projection near and far planes can be arbitrary, that is, near larger, near smaller,
//   both positive, both negative, one positive the other negative, etc.
template <typename T>
auto perspective(T fov, T nearPlane, T farPlane, T projNearPlane, T projFarPlane) {
    return perspective(abs(fov), vec<T, 1, false>{fov < T(0) ? T(-1) : T(1)}, nearPlane, farPlane, projNearPlane,
                       projFarPlane);
}

// Creates a 3D projection matrix.
// Field of view.
// FovX/FovY, so 1.777 for a 16:9 screen.
// Lower bound of the volume on the Y axis.
// Upper bound of the volume on the Y axis.
// Near plane is taken here after projection.
// Far plane is taken here after projection.
// The pre-projection near and far planes must be on the same side of the last axis,
//   i.e. both negative or both  positive.
// The post-projection near and far planes can be arbitrary, that is, near larger, near smaller,
//   both positive, both negative, one positive the other negative, etc.
// The _aspectRatio_ can be negative, in which case the second axis (i.e. Y) is inverted.
template <typename T>
auto perspective(T fov, T aspectRatio, T nearPlane, T farPlane, T projNearPlane, T projFarPlane) {
    return perspective(abs(fov), vec<T, 2, false>{fov < T(0) ? T(-1) : T(1), T(1) / aspectRatio}, nearPlane, farPlane,
                       projNearPlane, projFarPlane);
}

LSTD_END_NAMESPACE
