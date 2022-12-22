module;

#include "../common/namespace.h"

export module lstd.space_ship_replacement;

//
// This file defines the necessary types for the spaceship <=> operator in C++20 to work.
// partial_ordering, weak_ordering, strong_ordering, comparison_category_of
//
// Don't import this if you are including <compare> from the STL.
//

export namespace std {
using literal_zero = decltype(nullptr);

// These "pretty" enumerator names are safe since they reuse names of user-facing entities.
enum class compare_result_eq : char { EQUAL = 0 };  // -0.0 is equal to +0.0

enum class compare_result_ord : char {
    LESS = -1,
    GREATER = 1
};

enum class compare_result_unord : char { UNORDERED = -127 };

struct partial_ordering {
    char Value;

    explicit partial_ordering(const compare_result_eq value) : Value((char) value) {}
    explicit partial_ordering(const compare_result_ord value) : Value((char) value) {}
    explicit partial_ordering(const compare_result_unord value) : Value((char) value) {}

    static const partial_ordering less;
    static const partial_ordering equivalent;
    static const partial_ordering greater;
    static const partial_ordering unordered;

    friend bool operator==(const partial_ordering &, const partial_ordering &) = default;

    bool is_ordered() const { return Value != (char) compare_result_unord::UNORDERED; }
};

inline const partial_ordering partial_ordering::less(compare_result_ord::LESS);
inline const partial_ordering partial_ordering::equivalent(compare_result_eq::EQUAL);
inline const partial_ordering partial_ordering::greater(compare_result_ord::GREATER);
inline const partial_ordering partial_ordering::unordered(compare_result_unord::UNORDERED);

bool operator==(const partial_ordering value, literal_zero) { return value.Value == 0; }

bool operator<(const partial_ordering value, literal_zero) { return value.Value == (char) compare_result_ord::LESS; }
bool operator<(literal_zero, const partial_ordering value) { return 0 < value.Value; }

bool operator>(const partial_ordering value, literal_zero) { return value.Value > 0; }
bool operator>(literal_zero, const partial_ordering value) { return 0 > value.Value && value.is_ordered(); }

bool operator<=(const partial_ordering value, literal_zero) { return value.Value <= 0 && value.is_ordered(); }
bool operator<=(literal_zero, const partial_ordering value) { return 0 <= value.Value; }

bool operator>=(const partial_ordering value, literal_zero) { return value.Value >= 0; }
bool operator>=(literal_zero, const partial_ordering value) { return 0 >= value.Value && value.is_ordered(); }

partial_ordering operator<=>(const partial_ordering value, literal_zero) { return value; }
partial_ordering operator<=>(literal_zero, const partial_ordering value) { return partial_ordering{(compare_result_ord) -value.Value}; }

struct weak_ordering {
    char Value;

    explicit weak_ordering(const compare_result_eq value) : Value((char) value) {}
    explicit weak_ordering(const compare_result_ord value) : Value((char) value) {}

    static const weak_ordering less;
    static const weak_ordering equivalent;
    static const weak_ordering greater;

    operator partial_ordering() const { return partial_ordering{(compare_result_ord) Value}; }

    friend bool operator==(const weak_ordering &, const weak_ordering &) = default;
};

inline const weak_ordering weak_ordering::less(compare_result_ord::LESS);
inline const weak_ordering weak_ordering::equivalent(compare_result_eq::EQUAL);
inline const weak_ordering weak_ordering::greater(compare_result_ord::GREATER);

bool operator==(const weak_ordering value, literal_zero) { return value.Value == 0; }

bool operator<(const weak_ordering value, literal_zero) { return value.Value < 0; }
bool operator<(literal_zero, const weak_ordering value) { return 0 < value.Value; }

bool operator>(const weak_ordering value, literal_zero) { return value.Value > 0; }
bool operator>(literal_zero, const weak_ordering value) { return 0 > value.Value; }

bool operator<=(const weak_ordering value, literal_zero) { return value.Value <= 0; }
bool operator<=(literal_zero, const weak_ordering value) { return 0 <= value.Value; }

bool operator>=(const weak_ordering value, literal_zero) { return value.Value >= 0; }
bool operator>=(literal_zero, const weak_ordering value) { return 0 >= value.Value; }

weak_ordering operator<=>(const weak_ordering value, literal_zero) { return value; }
weak_ordering operator<=>(literal_zero, const weak_ordering value) { return weak_ordering{(compare_result_ord) -value.Value}; }

struct strong_ordering {
    char Value;

    explicit strong_ordering(const compare_result_eq value) : Value((char) value) {}
    explicit strong_ordering(const compare_result_ord value) : Value((char) value) {}

    static const strong_ordering less;
    static const strong_ordering equal;
    static const strong_ordering equivalent;
    static const strong_ordering greater;

    operator weak_ordering() const { return weak_ordering{(compare_result_ord) Value}; }
    operator partial_ordering() const { return partial_ordering{(compare_result_ord) Value}; }

    friend bool operator==(const strong_ordering &, const strong_ordering &) = default;
};

inline const strong_ordering strong_ordering::less(compare_result_ord::LESS);
inline const strong_ordering strong_ordering::equal(compare_result_eq::EQUAL);
inline const strong_ordering strong_ordering::equivalent(compare_result_eq::EQUAL);
inline const strong_ordering strong_ordering::greater(compare_result_ord::GREATER);

bool operator==(const strong_ordering value, literal_zero) { return value.Value == 0; }

bool operator<(const strong_ordering value, literal_zero) { return value.Value < 0; }
bool operator<(literal_zero, const strong_ordering value) { return 0 < value.Value; }

bool operator>(const strong_ordering value, literal_zero) { return value.Value > 0; }
bool operator>(literal_zero, const strong_ordering value) { return 0 > value.Value; }

bool operator<=(const strong_ordering value, literal_zero) { return value.Value <= 0; }
bool operator<=(literal_zero, const strong_ordering value) { return 0 <= value.Value; }

bool operator>=(const strong_ordering value, literal_zero) { return value.Value >= 0; }
bool operator>=(literal_zero, const strong_ordering value) { return 0 >= value.Value; }

strong_ordering operator<=>(const strong_ordering value, literal_zero) { return value; }
strong_ordering operator<=>(literal_zero, const strong_ordering value) { return strong_ordering{(compare_result_ord)(-value.Value)}; }
}  // namespace std

LSTD_BEGIN_NAMESPACE

export {
    using partial_ordering = std::partial_ordering;
    using weak_ordering = std::weak_ordering;
    using strong_ordering = std::strong_ordering;

    enum comparison_category : char {
        Comparison_Category_None = 1,
        Comparison_Category_Partial = 2,
        Comparison_Category_Weak = 4,
        Comparison_Category_Strong = 0,
    };

    // template <typename... Types>
    // inline unsigned char comparison_category_of = get_comparison_category{(get_comparison_category<Types> | ... | Comparison_Category_Strong)};

    template <typename T>
    inline unsigned char comparison_category_of = Comparison_Category_None;

    template <>
    inline unsigned char comparison_category_of<partial_ordering> = Comparison_Category_Partial;

    template <>
    inline unsigned char comparison_category_of<weak_ordering> = Comparison_Category_Weak;

    template <>
    inline unsigned char comparison_category_of<strong_ordering> = Comparison_Category_Strong;
}

LSTD_END_NAMESPACE
