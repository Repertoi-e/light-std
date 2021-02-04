#pragma once

#include "identity.h"

LSTD_BEGIN_NAMESPACE

template <typename T, s64 Dim, bool Packed>
struct scale_helper : non_copyable {
    vec<T, Dim, Packed> Scale;

    scale_helper(const vec<T, Dim, Packed> &scale) : Scale(scale) {}

    template <typename U, bool MPacked>
    operator mat<U, Dim + 1, Dim + 1, MPacked>() const {
        mat<U, Dim + 1, Dim + 1, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, Dim, Dim, MPacked>() const {
        mat<U, Dim, Dim, MPacked> m;
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
        m = identity();
        s64 i;
        for (i = 0; i < Scale.DIM; ++i) m(i, i) = (U) Scale[i];
        for (; i < min(R, C); ++i) m(i, i) = T(1);
    }
};

// Creates a scaling matrix.
// A vector containing the scales of respective axes.
// The vector's dimension must be less than or equal to the matrix dimension.
template <typename Vt, s64 Vdim, bool Vpacked>
auto scale(const vec<Vt, Vdim, Vpacked> &scale) {
    return scale_helper{scale};
}

// Creates a scaling matrix.
// A list of scalars corresponding to scaling on respective axes.
// The number of arguments must be less than or equal to the matrix dimension.
template <types::is_scalar... Args>
auto scale(Args... scales) {
    using PromotedT = decltype((0 + ... + scales));
    return scale_helper{vec<PromotedT, sizeof...(scales)>(scales...)};
}

LSTD_END_NAMESPACE
