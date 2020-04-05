#pragma once

#include "identity.h"

LSTD_BEGIN_NAMESPACE

template <typename T, s64 Dim, bool Packed>
struct translation_helper : non_copyable {
    vec<T, Dim, Packed> Translation;

    translation_helper(const vec<T, Dim, Packed> &translation) : Translation(translation) {}

    template <typename U, bool MPacked>
    operator mat<U, Dim + 1, Dim + 1, MPacked>() const {
        mat<U, Dim + 1, Dim + 1, MPacked> m = {no_init};
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, Dim + 1, Dim, MPacked>() const {
        mat<U, Dim + 1, Dim, MPacked> m = {no_init};
        set_impl(m);
        return m;
    }

   private:
    template <typename U, s64 R, s64 C, bool MPacked>
    void set_impl(mat<U, R, C, MPacked> &m) const {
        m = identity();
        For(range(Translation.Dim)) m(-1, it) = (U) Translation[it];
    }
};

/// Creates a translation matrix.
template <typename T, s64 Dim, bool Packed>
auto translation(const vec<T, Dim, Packed> &translation) {
    return translation_helper{translation};
}

namespace impl {
// C++ is dumb and we can't inline this
template <typename... Args>
constexpr bool are_scalar_v = (... && is_scalar_v<Args>);
}  // namespace impl

// Creates a translation matrix.
// A list of scalars that specify movement along repsective axes.
template <typename... Args, typename enable_if_t<impl::are_scalar_v<Args...>, s64> = 0>
auto translation(const Args &... coordinates) {
    using PromotedT = decltype((0 + ... + coordinates));
    return translation_helper{vec<PromotedT, sizeof...(coordinates)>(coordinates...)};
}

LSTD_END_NAMESPACE
