#pragma once

namespace std {
using _Literal_zero = decltype(nullptr);
using _Compare_t = signed char;

// These "pretty" enumerator names are safe since they reuse names of user-facing entities.
enum class _Compare_eq : _Compare_t { equal = 0,
                                      equivalent = equal };

enum class _Compare_ord : _Compare_t { less = -1,
                                       greater = 1 };

enum class _Compare_ncmp : _Compare_t { unordered = -127 };

// CLASS partial_ordering
class partial_ordering {
   public:
    constexpr explicit partial_ordering(const _Compare_eq _Value_) noexcept
        : _Value(static_cast<_Compare_t>(_Value_)) {}
    constexpr explicit partial_ordering(const _Compare_ord _Value_) noexcept
        : _Value(static_cast<_Compare_t>(_Value_)) {}
    constexpr explicit partial_ordering(const _Compare_ncmp _Value_) noexcept
        : _Value(static_cast<_Compare_t>(_Value_)) {}

    static const partial_ordering less;
    static const partial_ordering equivalent;
    static const partial_ordering greater;
    static const partial_ordering unordered;

    friend constexpr bool operator==(const partial_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value == 0;
    }

    friend constexpr bool operator==(const partial_ordering &, const partial_ordering &) noexcept = default;

    friend constexpr bool operator<(const partial_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value == static_cast<_Compare_t>(_Compare_ord::less);
    }

    friend constexpr bool operator>(const partial_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value > 0;
    }

    friend constexpr bool operator<=(const partial_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value <= 0 && _Val._Is_ordered();
    }

    friend constexpr bool operator>=(const partial_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value >= 0;
    }

    friend constexpr bool operator<(_Literal_zero, const partial_ordering _Val) noexcept {
        return 0 < _Val._Value;
    }

    friend constexpr bool operator>(_Literal_zero, const partial_ordering _Val) noexcept {
        return 0 > _Val._Value && _Val._Is_ordered();
    }

    friend constexpr bool operator<=(_Literal_zero, const partial_ordering _Val) noexcept {
        return 0 <= _Val._Value;
    }

    friend constexpr bool operator>=(_Literal_zero, const partial_ordering _Val) noexcept {
        return 0 >= _Val._Value && _Val._Is_ordered();
    }

    friend constexpr partial_ordering operator<=>(const partial_ordering _Val, _Literal_zero) noexcept {
        return _Val;
    }

    friend constexpr partial_ordering operator<=>(_Literal_zero, const partial_ordering _Val) noexcept {
        return partial_ordering{static_cast<_Compare_ord>(-_Val._Value)};
    }

   private:
    constexpr bool _Is_ordered() const noexcept {
        return _Value != static_cast<_Compare_t>(_Compare_ncmp::unordered);
    }

    _Compare_t _Value;
};

inline constexpr partial_ordering partial_ordering::less(_Compare_ord::less);
inline constexpr partial_ordering partial_ordering::equivalent(_Compare_eq::equivalent);
inline constexpr partial_ordering partial_ordering::greater(_Compare_ord::greater);
inline constexpr partial_ordering partial_ordering::unordered(_Compare_ncmp::unordered);

// CLASS weak_ordering
class weak_ordering {
   public:
    constexpr explicit weak_ordering(const _Compare_eq _Value_) noexcept
        : _Value(static_cast<_Compare_t>(_Value_)) {}
    constexpr explicit weak_ordering(const _Compare_ord _Value_) noexcept
        : _Value(static_cast<_Compare_t>(_Value_)) {}

    static const weak_ordering less;
    static const weak_ordering equivalent;
    static const weak_ordering greater;

    constexpr operator partial_ordering() const noexcept {
        return partial_ordering{static_cast<_Compare_ord>(_Value)};
    }

    friend constexpr bool operator==(const weak_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value == 0;
    }

    friend constexpr bool operator==(const weak_ordering &, const weak_ordering &) noexcept = default;

    friend constexpr bool operator<(const weak_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value < 0;
    }

    friend constexpr bool operator>(const weak_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value > 0;
    }

    friend constexpr bool operator<=(const weak_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value <= 0;
    }

    friend constexpr bool operator>=(const weak_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value >= 0;
    }

    friend constexpr bool operator<(_Literal_zero, const weak_ordering _Val) noexcept {
        return 0 < _Val._Value;
    }

    friend constexpr bool operator>(_Literal_zero, const weak_ordering _Val) noexcept {
        return 0 > _Val._Value;
    }

    friend constexpr bool operator<=(_Literal_zero, const weak_ordering _Val) noexcept {
        return 0 <= _Val._Value;
    }

    friend constexpr bool operator>=(_Literal_zero, const weak_ordering _Val) noexcept {
        return 0 >= _Val._Value;
    }

    friend constexpr weak_ordering operator<=>(const weak_ordering _Val, _Literal_zero) noexcept {
        return _Val;
    }

    friend constexpr weak_ordering operator<=>(_Literal_zero, const weak_ordering _Val) noexcept {
        return weak_ordering{static_cast<_Compare_ord>(-_Val._Value)};
    }

   private:
    _Compare_t _Value;
};

inline constexpr weak_ordering weak_ordering::less(_Compare_ord::less);
inline constexpr weak_ordering weak_ordering::equivalent(_Compare_eq::equivalent);
inline constexpr weak_ordering weak_ordering::greater(_Compare_ord::greater);

// CLASS strong_ordering
class strong_ordering {
   public:
    constexpr explicit strong_ordering(const _Compare_eq _Value_) noexcept
        : _Value(static_cast<_Compare_t>(_Value_)) {}
    constexpr explicit strong_ordering(const _Compare_ord _Value_) noexcept
        : _Value(static_cast<_Compare_t>(_Value_)) {}

    static const strong_ordering less;
    static const strong_ordering equal;
    static const strong_ordering equivalent;
    static const strong_ordering greater;

    constexpr operator partial_ordering() const noexcept {
        return partial_ordering{static_cast<_Compare_ord>(_Value)};
    }

    constexpr operator weak_ordering() const noexcept {
        return weak_ordering{static_cast<_Compare_ord>(_Value)};
    }

    friend constexpr bool operator==(const strong_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value == 0;
    }

    friend constexpr bool operator==(const strong_ordering &, const strong_ordering &) noexcept = default;

    friend constexpr bool operator<(const strong_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value < 0;
    }

    friend constexpr bool operator>(const strong_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value > 0;
    }

    friend constexpr bool operator<=(const strong_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value <= 0;
    }

    friend constexpr bool operator>=(const strong_ordering _Val, _Literal_zero) noexcept {
        return _Val._Value >= 0;
    }

    friend constexpr bool operator<(_Literal_zero, const strong_ordering _Val) noexcept {
        return 0 < _Val._Value;
    }

    friend constexpr bool operator>(_Literal_zero, const strong_ordering _Val) noexcept {
        return 0 > _Val._Value;
    }

    friend constexpr bool operator<=(_Literal_zero, const strong_ordering _Val) noexcept {
        return 0 <= _Val._Value;
    }

    friend constexpr bool operator>=(_Literal_zero, const strong_ordering _Val) noexcept {
        return 0 >= _Val._Value;
    }

    friend constexpr strong_ordering operator<=>(const strong_ordering _Val, _Literal_zero) noexcept {
        return _Val;
    }

    friend constexpr strong_ordering operator<=>(_Literal_zero, const strong_ordering _Val) noexcept {
        return strong_ordering{static_cast<_Compare_ord>(-_Val._Value)};
    }

   private:
    _Compare_t _Value;
};

inline constexpr strong_ordering strong_ordering::less(_Compare_ord::less);
inline constexpr strong_ordering strong_ordering::equal(_Compare_eq::equal);
inline constexpr strong_ordering strong_ordering::equivalent(_Compare_eq::equivalent);
inline constexpr strong_ordering strong_ordering::greater(_Compare_ord::greater);

// FUNCTION is_eq
constexpr bool is_eq(const partial_ordering _Comp) noexcept {
    return _Comp == 0;
}

// FUNCTION is_neq
constexpr bool is_neq(const partial_ordering _Comp) noexcept {
    return _Comp != 0;
}

// FUNCTION is_lt
constexpr bool is_lt(const partial_ordering _Comp) noexcept {
    return _Comp < 0;
}

// FUNCTION is_lteq
constexpr bool is_lteq(const partial_ordering _Comp) noexcept {
    return _Comp <= 0;
}

// FUNCTION is_gt
constexpr bool is_gt(const partial_ordering _Comp) noexcept {
    return _Comp > 0;
}

// FUNCTION is_gteq
constexpr bool is_gteq(const partial_ordering _Comp) noexcept {
    return _Comp >= 0;
}

// ALIAS TEMPLATE common_comparison_category_t
enum _Comparison_category : unsigned char {
    _Comparison_category_none = 1,
    _Comparison_category_partial = 2,
    _Comparison_category_weak = 4,
    _Comparison_category_strong = 0,
};

template <class... _Types>
inline constexpr unsigned char _Classify_category =
    _Comparison_category{(_Classify_category<_Types> | ... | _Comparison_category_strong)};
template <class _Ty>
inline constexpr unsigned char _Classify_category<_Ty> = _Comparison_category_none;
template <>
inline constexpr unsigned char _Classify_category<partial_ordering> = _Comparison_category_partial;
template <>
inline constexpr unsigned char _Classify_category<weak_ordering> = _Comparison_category_weak;
template <>
inline constexpr unsigned char _Classify_category<strong_ordering> = _Comparison_category_strong;
}  // namespace std

/*
template <class... _Types>
using common_comparison_category_t =
    conditional_t<(_Classify_category<_Types...> & _Comparison_category_none) != 0, void,
                  conditional_t<(_Classify_category<_Types...> & _Comparison_category_partial) != 0, partial_ordering,
                                conditional_t<(_Classify_category<_Types...> & _Comparison_category_weak) != 0, weak_ordering,
                                              strong_ordering>>>;

template <class... _Types>
struct common_comparison_category {
    using type = common_comparison_category_t<_Types...>;
};

*/