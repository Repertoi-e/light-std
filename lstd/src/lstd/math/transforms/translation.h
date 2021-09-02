#pragma once

#include "identity.h"

LSTD_BEGIN_NAMESPACE

template <typename T, s64 Dim, bool Packed>
struct translation_helper : non_copyable {
    vec<T, Dim, Packed> Translation;

    translation_helper(const vec<T, Dim, Packed> &translation)
        : Translation(translation) {
    }

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

private:
    template <typename U, s64 R, s64 C, bool MPacked>
    void set_impl(mat<U, R, C, MPacked> &m) const {
        m = identity();
        For(range(Translation.DIM)) m(-1, it) = (U) Translation[it];
    }
};

/// Creates a translation matrix.
template <typename T, s64 Dim, bool Packed>
auto translation(const vec<T, Dim, Packed> &translation) {
    return translation_helper{translation};
}

// Creates a translation matrix.
// A list of scalars that specify movement along repsective axes.
template <types::is_scalar... Args>
auto translation(const Args &... coordinates) {
    using PromotedT = decltype((0 + ... + coordinates));
    return translation_helper{vec<PromotedT, sizeof...(coordinates)>(coordinates...)};
}

LSTD_END_NAMESPACE
