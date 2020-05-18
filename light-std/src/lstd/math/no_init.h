#pragma once

#include "../internal/common.h"

LSTD_BEGIN_NAMESPACE

struct no_init_t {};
constexpr no_init_t no_init = {};

namespace impl {
template <typename MathT, size_t... Is>
stack_array<MathT, sizeof...(Is)> make_stack_array_helper(index_sequence<Is...>) {
    return {((void) Is, MathT(no_init))...};
}
}  // namespace impl

template <typename MathT, size_t N>
stack_array<MathT, N> make_stack_array_of_uninitialized_math_type() {
    return impl::make_stack_array_helper<MathT>(make_index_sequence<N>());
}

LSTD_END_NAMESPACE
