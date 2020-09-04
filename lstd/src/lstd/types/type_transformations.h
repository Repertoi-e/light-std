#pragma once

#include "type_info.h"

LSTD_BEGIN_NAMESPACE

namespace types {
//
// Transformations:
//

// Doesn't handle enums!
template <typename T>
struct make_signed {
    using type = T;
};

#define MAKE_SIGNED_HELPER(uns, s)           \
    template <>                              \
    struct make_signed<uns> {                \
        using type = s;                      \
    };                                       \
                                             \
    template <>                              \
    struct make_signed<const uns> {          \
        using type = const s;                \
    };                                       \
                                             \
    template <>                              \
    struct make_signed<volatile uns> {       \
        using type = volatile s;             \
    };                                       \
                                             \
    template <>                              \
    struct make_signed<const volatile uns> { \
        using type = const volatile s;       \
    };
MAKE_SIGNED_HELPER(u8, s8)
MAKE_SIGNED_HELPER(u16, s16)
MAKE_SIGNED_HELPER(u32, s32)
MAKE_SIGNED_HELPER(u64, s64)
#if defined _NATIVE_WCHAR_T_DEFINED
MAKE_SIGNED_HELPER(wchar_t, s16)
#endif
MAKE_SIGNED_HELPER(char8_t, s8)
MAKE_SIGNED_HELPER(char16_t, s16)
MAKE_SIGNED_HELPER(char32_t, s32)

#undef MAKE_SIGNED_HELPER

template <typename T>
using make_signed_t = typename make_signed<T>::type;

// Doesn't handle enums!
template <typename T>
struct make_unsigned {
    using type = T;
};

#define MAKE_UNSIGNED_HELPER(uns, s)           \
    template <>                                \
    struct make_unsigned<uns> {                \
        using type = s;                        \
    };                                         \
                                               \
    template <>                                \
    struct make_unsigned<const uns> {          \
        using type = const s;                  \
    };                                         \
                                               \
    template <>                                \
    struct make_unsigned<volatile uns> {       \
        using type = volatile s;               \
    };                                         \
                                               \
    template <>                                \
    struct make_unsigned<const volatile uns> { \
        using type = const volatile s;         \
    };
MAKE_UNSIGNED_HELPER(s8, u8)
MAKE_UNSIGNED_HELPER(s16, u16)
MAKE_UNSIGNED_HELPER(s32, u32)
MAKE_UNSIGNED_HELPER(s64, u64)

#undef MAKE_UNSIGNED_HELPER

template <typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

template <typename T>
struct remove_pointer {
    using type = T;
};

template <typename T>
struct remove_pointer<T *> {
    using type = T;
};

template <typename T>
using remove_pointer_t = typename remove_pointer<remove_cv_t<T>>::type;

template <typename T>
struct add_pointer {
    using type = typename remove_reference<T>::type *;
};

template <typename T>
using add_pointer_t = typename add_pointer<T>::type;

// The remove_extent transformation trait removes a dimension from an array.
// For a given non-array type T, remove_extent<T>::type is equivalent to T.
// For a given array type T[N], remove_extent<T[N]>::type is equivalent to T.
// For a given array type const T[N], remove_extent<const T[N]>::type is equivalent to const T.
// For example, given a multi-dimensional array type T[M][N], remove_extent<T[M][N]>::type is equivalent to T[N].
template <typename T>
struct remove_extent {
    using type = T;
};

template <typename T>
struct remove_extent<T[]> {
    using type = T;
};

template <typename T, s64 N>
struct remove_extent<T[N]> {
    using type = T;
};

template <typename T>
using remove_extent_t = typename remove_extent<T>::type;

// The remove_all_extents transformation trait removes all dimensions from an array.
// For a given non-array type T, remove_all_extents<T>::type is equivalent to T.
// For a given array type T[N], remove_all_extents<T[N]>::type is equivalent to T.
// For a given array type const T[N], remove_all_extents<const T[N]>::type is equivalent to const T.
// For example, given a multi-dimensional array type T[M][N], remove_all_extents<T[M][N]>::type is equivalent to T.
template <typename T>
struct remove_all_extents {
    using type = T;
};

template <typename T, s64 N>
struct remove_all_extents<T[N]> {
    using type = typename remove_all_extents<T>::type;
};

template <typename T>
struct remove_all_extents<T[]> {
    using type = typename remove_all_extents<T>::type;
};

template <typename T>
using remove_all_extents_t = typename remove_all_extents<T>::type;

// :CopyMemory
extern void (*copy_memory)(void *dest, const void *src, s64 num);

// Safely converts between unrelated types that have a binary equivalency.
// This appoach is required by strictly conforming C++ compilers because
// directly using a C or C++ cast between unrelated types is fraught with
// the possibility of undefined runtime behavior due to type aliasing.
//
// Example usage:
//    float f32 = 1.234f;
//    uint32_t n32 = bit_cast<uint32_t>(f32);
template <typename DestType, typename SourceType>
DestType bit_cast(const SourceType &sourceValue) {
    static_assert(sizeof(DestType) == sizeof(SourceType));

    if constexpr (alignof(DestType) == alignof(SourceType)) {
        union {
            SourceType sourceValue;
            DestType destValue;
        } u;
        u.sourceValue = sourceValue;
        return u.destValue;
    } else {
        DestType destValue;
        copy_memory(&destValue, &sourceValue, sizeof(DestType));
        return destValue;
    }
}

// Converts the type T to its decayed equivalent. That means doing
// lvalue to rvalue, array to pointer, function to pointer conversions,
// and removal of const and volatile.
// This is the type conversion silently applied by the compiler to
// all function arguments when passed by value.
template <typename T>
struct decay {
    using U = remove_reference_t<T>;

    using type = select_t<(bool) is_array<U>,
                          remove_extent_t<U> *,
                          select_t<(bool) is_function<U>,
                                   add_pointer_t<U>,
                                   remove_cv_t<U>>>;
};

template <typename T>
using decay_t = typename decay<T>::type;

// Determines the common type among all types T..., that is the type all T...
// can be implicitly converted to.
//
// It is intended that this be specialized by the user for cases where it
// is useful to do so. Example specialization:
//     template <typename Class1, typename Class2>
//     struct common_type<MyClass1, MyClass2>{ using type = MyBaseClassB; };
template <typename... T>
struct common_type;

template <typename T>
struct common_type<T> {
    using type = decay_t<T>;
};

template <typename T, typename U>
struct common_type<T, U> {
    using type = decay_t<decltype(true ? declval<T>() : declval<U>())>;
};

template <typename T, typename U, typename... V>
struct common_type<T, U, V...> {
    using type = typename common_type<typename common_type<T, U>::type, V...>::type;
};

template <typename... T>
using common_type_t = typename common_type<T...>::type;

template <typename... Types>
using common_comparison_category_t = select_t<(comparison_category_of<Types...> & Comparison_Category_None) != 0, void,
                                              select_t<(comparison_category_of<Types...> & Comparison_Category_Partial) != 0, partial_ordering,
                                                       select_t<(comparison_category_of<Types...> & Comparison_Category_Weak) != 0, weak_ordering,
                                                                strong_ordering>>>;

template <typename... Types>
struct common_comparison_category {
    using type = common_comparison_category_t<Types...>;
};
}  // namespace types

LSTD_END_NAMESPACE
