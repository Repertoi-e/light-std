#pragma once

#include "../internal/common.h"

LSTD_BEGIN_NAMESPACE

struct no_init_t {};

// :MathTypesNoInit By default we zero-init but you can call a special constructor with this value which doesn't initialize the object.
// @TODO: We still have to make this work with data-structures like arrays or tables... But how? 
constexpr no_init_t no_init = {};

namespace impl {
template <typename MathT, s64... Is>
stack_array<MathT, sizeof...(Is)> make_stack_array_helper(type::index_sequence<Is...>) {
    return {((void) Is, MathT(no_init))...};
}
}  // namespace impl

template <typename MathT, s64 N>
stack_array<MathT, N> make_stack_array_of_uninitialized_math_type() {
    return impl::make_stack_array_helper<MathT>(type::make_index_sequence<N>());
}

LSTD_END_NAMESPACE
