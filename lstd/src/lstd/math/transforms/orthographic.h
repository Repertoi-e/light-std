#pragma once

#include "identity.h"

LSTD_BEGIN_NAMESPACE

template <typename T, s64 Dim, bool Packed>
struct ortographic_helper : non_copyable {
    using VectorT = vec<T, Dim, Packed>;

    vec<T, Dim, Packed> MinBounds, MaxBounds;
    T ProjNearPlane, ProjFarPlane;

    ortographic_helper(const VectorT &minBounds, const VectorT &maxBounds, T projNearPlane, T projFarPlane)
        : MinBounds(minBounds), MaxBounds(maxBounds), ProjNearPlane(projNearPlane), ProjFarPlane(projFarPlane) {}

    template <typename U, bool MPacked>
    operator mat<U, Dim + 1, Dim + 1, MPacked>() const {
        mat<U, Dim + 1, Dim + 1, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, Dim + 1, Dim, MPacked>() const {
        mat<U, Dim + 1, Dim, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, s64 R, s64 C, bool MPacked>
    void set_impl(mat<U, R, C, MPacked> &m) const {
        using VectorT = vec<T, Dim, false>;

        VectorT volumeSize = MaxBounds - MinBounds;

        VectorT scale = T(2) / volumeSize;
        scale[scale.DIM - 1] *= T(0.5) * (ProjFarPlane - ProjNearPlane);

        VectorT offset = -(MaxBounds + MinBounds) / T(2) * scale;
        offset[offset.DIM - 1] += (ProjFarPlane + ProjNearPlane) / 2;

        m = identity();
        For(range(scale.DIM)) {
            m(it, it) = scale[it];
            m(scale.DIM, it) = offset[it];
        }
    }
};

// Creates an orthographics projection matrix.
// The volume before projection is an axis-aligned hypercube and it is projected onto a unit hypercube
// After projection, all axes range from -1 to 1, except for the last axis, which is specified explicitly.
//
// minBounds - The "left" corner of the hypercube.
// maxBoubds - The "right" corner of the hypercube.
// projNearPlane - The lower bound of the last axis of the projected volume (Z axis in 3D).
// projFarPlane - The upper bound of the last axis of the projected volume (Z axis in 3D).
template <typename T, s64 Dim, bool Packed>
auto orthographic(const vec<T, Dim, Packed> &minBounds, const vec<T, Dim, Packed> &maxBounds, T projNearPlane, T projFarPlane) {
    if constexpr (types::is_integral<T>) {
        using VectorT = vec<f32, Dim, false>;
        return ortographic_helper(VectorT(minBounds), VectorT(maxBounds), f32(projNearPlane), f32(projFarPlane));
    } else {
        return ortographic_helper(minBounds, maxBounds, projNearPlane, projFarPlane);
    }
}

LSTD_END_NAMESPACE
