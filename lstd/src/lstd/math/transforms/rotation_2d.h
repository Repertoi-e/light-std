#pragma once

#include "../mat.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
struct rotation_2d_helper : non_copyable {
    T Angle;

    rotation_2d_helper(T angle) : Angle(angle) {}

    template <typename U, bool MPacked>
    operator mat<U, 3, 3, MPacked>() const {
        mat<U, 3, 3, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, 2, 2, MPacked>() const {
        mat<U, 2, 2, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, 3, 2, MPacked>() const {
        mat<U, 3, 2, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, s64 RC, s64 CC, bool MPacked>
    void set_impl(mat<U, RC, CC, MPacked> &m) const {
        T C = (T) Math_Cos_flt32(Angle);
        T S = (T) Math_Sin_flt32(Angle);

        // Indices according to follow vector order
        m(0, 0) = U(C);
        m(0, 1) = U(S);
        m(1, 0) = U(-S);
        m(1, 1) = U(C);

        // Rest
        For_as(j, range(m.C)) { For_as(i, range(j < 2 ? 2 : 0, m.R)) m(i, j) = U(j == i); }
    }
};

// Creates a 2D rotation matrix.
// Counter-clockwise angle in radians.
template <typename T>
auto rotation(T angle) {
    return rotation_2d_helper{angle};
}

LSTD_END_NAMESPACE
