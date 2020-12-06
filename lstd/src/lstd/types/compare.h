#pragma once

#include "../internal/namespace.h"

//
// This file defines the necessary types for the spaceship <=> operator in C++20 to work.
// partial_ordering, weak_ordering, strong_ordering, comparison_category_of
//

// @DependencyCleanup We still have dependencies on the math library which prevents us from not including STD headers.
// As far as I can tell every header in the STD includes some common file which contains the def for these comparison types.
// This also applies for initializer_list.
#define LSTD_DONT_DEFINE_INITIALIZER_LIST

#if not defined LSTD_DONT_DEFINE_INITIALIZER_LIST
LSTD_BEGIN_NAMESPACE

namespace std {
using literal_zero = decltype(null);

// These "pretty" enumerator names are safe since they reuse names of user-facing entities.
enum class compare_result_eq : s8 { EQUAL = 0 };  // -0.0 is equal to +0.0

enum class compare_result_ord : s8 {
    LESS = -1,
    GREATER = 1
};

enum class compare_result_unord : s8 { UNORDERED = -127 };

struct partial_ordering {
    s8 Value;

    constexpr explicit partial_ordering(const compare_result_eq value) : Value((s8) value) {}
    constexpr explicit partial_ordering(const compare_result_ord value) : Value((s8) value) {}
    constexpr explicit partial_ordering(const compare_result_unord value) : Value((s8) value) {}

    static const partial_ordering less;
    static const partial_ordering equivalent;
    static const partial_ordering greater;
    static const partial_ordering unordered;

    friend constexpr bool operator==(const partial_ordering &, const partial_ordering &) = default;

    constexpr bool is_ordered() const { return Value != (s8) compare_result_unord::UNORDERED; }
};

inline constexpr partial_ordering partial_ordering::less(compare_result_ord::LESS);
inline constexpr partial_ordering partial_ordering::equivalent(compare_result_eq::EQUAL);
inline constexpr partial_ordering partial_ordering::greater(compare_result_ord::GREATER);
inline constexpr partial_ordering partial_ordering::unordered(compare_result_unord::UNORDERED);

constexpr bool operator==(const partial_ordering value, literal_zero) { return value.Value == 0; }

constexpr bool operator<(const partial_ordering value, literal_zero) { return value.Value == (s8) compare_result_ord::LESS; }
constexpr bool operator<(literal_zero, const partial_ordering value) { return 0 < value.Value; }

constexpr bool operator>(const partial_ordering value, literal_zero) { return value.Value > 0; }
constexpr bool operator>(literal_zero, const partial_ordering value) { return 0 > value.Value && value.is_ordered(); }

constexpr bool operator<=(const partial_ordering value, literal_zero) { return value.Value <= 0 && value.is_ordered(); }
constexpr bool operator<=(literal_zero, const partial_ordering value) { return 0 <= value.Value; }

constexpr bool operator>=(const partial_ordering value, literal_zero) { return value.Value >= 0; }
constexpr bool operator>=(literal_zero, const partial_ordering value) { return 0 >= value.Value && value.is_ordered(); }

constexpr partial_ordering operator<=>(const partial_ordering value, literal_zero) { return value; }
constexpr partial_ordering operator<=>(literal_zero, const partial_ordering value) { return partial_ordering{(compare_result_ord) -value.Value}; }

struct weak_ordering {
    s8 Value;

    constexpr explicit weak_ordering(const compare_result_eq value) : Value((s8) value) {}
    constexpr explicit weak_ordering(const compare_result_ord value) : Value((s8) value) {}

    static const weak_ordering less;
    static const weak_ordering equivalent;
    static const weak_ordering greater;

    constexpr operator partial_ordering() const { return partial_ordering{(compare_result_ord) Value}; }

    friend constexpr bool operator==(const weak_ordering &, const weak_ordering &) = default;
};

inline constexpr weak_ordering weak_ordering::less(compare_result_ord::LESS);
inline constexpr weak_ordering weak_ordering::equivalent(compare_result_eq::EQUAL);
inline constexpr weak_ordering weak_ordering::greater(compare_result_ord::GREATER);

constexpr bool operator==(const weak_ordering value, literal_zero) { return value.Value == 0; }

constexpr bool operator<(const weak_ordering value, literal_zero) { return value.Value < 0; }
constexpr bool operator<(literal_zero, const weak_ordering value) { return 0 < value.Value; }

constexpr bool operator>(const weak_ordering value, literal_zero) { return value.Value > 0; }
constexpr bool operator>(literal_zero, const weak_ordering value) { return 0 > value.Value; }

constexpr bool operator<=(const weak_ordering value, literal_zero) { return value.Value <= 0; }
constexpr bool operator<=(literal_zero, const weak_ordering value) { return 0 <= value.Value; }

constexpr bool operator>=(const weak_ordering value, literal_zero) { return value.Value >= 0; }
constexpr bool operator>=(literal_zero, const weak_ordering value) { return 0 >= value.Value; }

constexpr weak_ordering operator<=>(const weak_ordering value, literal_zero) { return value; }
constexpr weak_ordering operator<=>(literal_zero, const weak_ordering value) { return weak_ordering{(compare_result_ord) -value.Value}; }

struct strong_ordering {
    s8 Value;

    constexpr explicit strong_ordering(const compare_result_eq value) : Value((s8) value) {}
    constexpr explicit strong_ordering(const compare_result_ord value) : Value((s8) value) {}

    static const strong_ordering less;
    static const strong_ordering equal;
    static const strong_ordering equivalent;
    static const strong_ordering greater;

    constexpr operator weak_ordering() const { return weak_ordering{(compare_result_ord) Value}; }
    constexpr operator partial_ordering() const { return partial_ordering{(compare_result_ord) Value}; }

    friend constexpr bool operator==(const strong_ordering &, const strong_ordering &) = default;
};

inline constexpr strong_ordering strong_ordering::less(compare_result_ord::LESS);
inline constexpr strong_ordering strong_ordering::equal(compare_result_eq::EQUAL);
inline constexpr strong_ordering strong_ordering::equivalent(compare_result_eq::EQUAL);
inline constexpr strong_ordering strong_ordering::greater(compare_result_ord::GREATER);

constexpr bool operator==(const strong_ordering value, literal_zero) { return value.Value == 0; }

constexpr bool operator<(const strong_ordering value, literal_zero) { return value.Value < 0; }
constexpr bool operator<(literal_zero, const strong_ordering value) { return 0 < value.Value; }

constexpr bool operator>(const strong_ordering value, literal_zero) { return value.Value > 0; }
constexpr bool operator>(literal_zero, const strong_ordering value) { return 0 > value.Value; }

constexpr bool operator<=(const strong_ordering value, literal_zero) { return value.Value <= 0; }
constexpr bool operator<=(literal_zero, const strong_ordering value) { return 0 <= value.Value; }

constexpr bool operator>=(const strong_ordering value, literal_zero) { return value.Value >= 0; }
constexpr bool operator>=(literal_zero, const strong_ordering value) { return 0 >= value.Value; }

constexpr strong_ordering operator<=>(const strong_ordering value, literal_zero) { return value; }
constexpr strong_ordering operator<=>(literal_zero, const strong_ordering value) { return strong_ordering{(compare_result_ord)(-value.Value)}; }
}  // namespace std

using partial_ordering = std::partial_ordering;
using weak_ordering = std::weak_ordering;
using strong_ordering = std::strong_ordering;

enum comparison_category : u8 {
    Comparison_Category_None = 1,
    Comparison_Category_Partial = 2,
    Comparison_Category_Weak = 4,
    Comparison_Category_Strong = 0,
};

// template <typename... Types>
// inline constexpr unsigned char comparison_category_of = get_comparison_category{(get_comparison_category<Types> | ... | Comparison_Category_Strong)};

template <typename T>
inline constexpr byte comparison_category_of = Comparison_Category_None;

template <>
inline constexpr byte comparison_category_of<partial_ordering> = Comparison_Category_Partial;

template <>
inline constexpr byte comparison_category_of<weak_ordering> = Comparison_Category_Weak;

template <>
inline constexpr byte comparison_category_of<strong_ordering> = Comparison_Category_Strong;

LSTD_END_NAMESPACE
#endif
