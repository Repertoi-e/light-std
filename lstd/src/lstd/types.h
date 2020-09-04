#pragma once

// #include <concepts> // XXXXXXXXXXXX

#include "types/basic_types.h"
#include "types/math_types.h"
#include "types/type_info.h"

/// Provides definitions for common types as well as most stuff from the std headers: <type_traits> <limits>
/// as well as some stuff from <utility> <concepts>

//
// Min/max values are also defined (S8_MIN, S8_MAX, etc...)
//
// What's missing from std:
// - conditional (select is equivalent)
// - is_trivially_default_constructible (is_trivially_constructible is equivalent)
// - is_default_constructible (is_constructible is equivalent)
// - is_explicitly_convertible (is_convertible is equivalent)
// - numeric_info (numeric_info is equivalent)
// - float_round_style:: and float_denorm_style:: (here, but naming is different)
//
// - is_nothrow_convertible (we don't care about exceptions and don't use them anywhere in this library)
// - has_nothrow_constructor
// - has_nothrow_copy
// - has_nothrow_assign
// - is_nothrow_constructible
// - is_nothrow_default_constructible
// - is_nothrow_copy_constructible
// - is_nothrow_copy_constructible
// - is_nothrow_assignable
// - is_nothrow_copy_assignable
// - is_nothrow_move_assignable
// - is_nothrow_destructible
// - is_nothrow_default_constructible
// - is_nothrow_move_constructible
//
// - is_polymorphic (we don't care about virtual functions and don't use them anywhere in this library)
// - has_virtual_destructor
// - is_abstract
// - is_final
//
// - any type def ending _type ends with _t (e.g. false_t, true_t instead of false_type, true_type)
//
// What's implemented here but not part of std:
// - is_array_of_known_bounds
// - is_array_of_unknown_bounds
// - is_equal_comparable
// - is_less_comparable
// - all
// - any
// - concat_type_list
// - repeat_type
// - merge_integer_sequence
// - reverse_integer_sequence

LSTD_BEGIN_NAMESPACE

namespace types {

// ==
template <typename T, typename U>
concept equal_comparable = requires(T a, U b) { a == b; };

// <
template <typename T, typename U>
concept less_comparable = requires(T a, U b) { a < b; };

}  // namespace types
