#pragma once

#include "compare.h"
#include "ieee.h"
#include "types.h"

//
// This file defines the following types:
// - integral_constant (a struct with a type and compile-time value, usually in the context of integrals)
// - true_t, false_t (integral_constant<bool, true/false>, this is == to std::true_type, std::false_type)
// - unused (a dummy type used in templates because C++ is dumb)
//
// Helper templates:
// - select (select from two types based on a condition, this is == to std::conditional)
// - enable_if (used for SNFIAE or whatever it is, not reccommended... use concepts or if constexpr)
//
// Concepts:
// - is_same (checks if two types are the same)
// - is_const, is_volatile, is_reference, is_rvalue_reference, is_pointer, is_function (doesn't include member functions)
// - is_void, is_null (decltype(null))
// - is_integral, is_signed_integral, is_unsigned_integral, is_floating_point, is_arithmetic
// - is_fundamental, is_union, is_class, is_enum, is_object
// - is_member_pointer, is_member_object_pointer, is_member_function_pointer
// - is_scalar
// - is_constructible, is_convertible
//
// Info about arrays:
// - rank (returns the number of dimensions of the array, e.g s32[][][] -> 3)
// - extent_v (returns the size of the nth extent of an array, e.g. extent_v<s32[2][4][9], 1> -> 4)
// - is_array
// - is_array_of_known_bounds, is_array_of_unknown_bounds (not part of the C++ std)
//
// Transformations:
// - remove_const, remove_volatile, remove_cv, remove_reference, remove_cvref, add_const, add_volatile, add_reference, add_rvalue_reference
// - make_signed/make_unsigned (doesn't handle enums!)
// - underlying_type (returns the underlying integral type on the enum)
//
// - remove_pointer/add_pointer
// - remove_extent (for arrays, e.g. s32[][] -> int[])
// - remove_all_extents (for arrays, e.g. s32[][] -> s32)
//
// - decay (applies lvalue-to-rvalue, array-to-pointer, function-to-pointer implicit conversions to the type T, removes cv-qualifiers)
// - common_type (determines the common type among all types T..., that is the type all T... can be implicitly converted to, can be explictly specialized by the user)
//
// - bit_cast (converts one type to another by reinterpreting the bits, uses an union if the two types have the same alingment, otherwise calls copy_memory)
//

LSTD_BEGIN_NAMESPACE

namespace types {

// This is essentially a utility base struct for defining properties as both struct constants and as types.
template <typename T, T Value>
struct integral_constant {
    static constexpr T value = Value;

    using value_t = T;
    using type = integral_constant<T, Value>;

    constexpr operator value_t() const { return value; }
    constexpr value_t operator()() const { return value; }
};

using true_t = integral_constant<bool, true>;    // == to std::true_type
using false_t = integral_constant<bool, false>;  // == to std::false_type

// Used to denote a special template argument that means it's an unused argument
struct unused {};

// This is used to declare a type from one of two type options.
// The result is based on the condition type.
//
// e.g:
//    using chosen_t = select_t<Condition, ChoiceAType, ChoiceBType>;
//
template <bool Condition, typename ConditionIsTrueType, typename ConditionIsFalseType>
struct select {
    using type = ConditionIsTrueType;
};

template <typename ConditionIsTrueType, typename ConditionIsFalseType>
struct select<false, ConditionIsTrueType, ConditionIsFalseType> {
    using type = ConditionIsFalseType;
};

template <bool Condition, typename ConditionIsTrueType, typename ConditionIsFalseType>
using select_t = typename select<Condition, ConditionIsTrueType, ConditionIsFalseType>::type;

// Similar to select but unilaterally selects the first type.
template <typename T, typename = unused, typename = unused>
struct first_select {
    using type = T;
};

template <typename T, typename = unused, typename = unused>
using first_select_t = typename first_select<T>::type;

// Used for SNFIAE or whatever it is. Not reccommended... use concepts or if constexpr.
template <bool B, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
    using type = T;
};

template <bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

//
// Checks if two types are the same
//
template <typename T, typename U>
struct same_helper : public false_t {};

template <typename T>
struct same_helper<T, T> : public true_t {};

template <typename T, typename U>
concept is_same = same_helper<T, U>::value;

// Are the decayed versions of "T" and "U" the same basic type?
// Gets around the fact that is_same will treat, say "bool" and "bool&" as different types.
template <typename T, typename U>
concept is_same_decayed = same_helper<decay_t<T>, decay_t<U>>::value;

// Checks if T has const-qualification
template <typename T>
struct is_const_helper_1 : public false_t {};
template <typename T>
struct is_const_helper_1<volatile T *> : public true_t {};
template <typename T>
struct is_const_helper_1<const volatile T *> : public true_t {};

template <typename T>
struct is_const_helper_2 : public is_const_helper_1<T *> {};

template <typename T>
struct is_const_helper_2<T &> : public false_t {};  // Note here that T is const, not the reference to T. So is_const is false.

template <typename T>
concept is_const = is_const_helper_2<T>::value;

// Checks if T has volatile-qualification
template <typename T>
struct is_volatile_helper_1 : public false_t {};
template <typename T>
struct is_volatile_helper_1<const T *> : public true_t {};
template <typename T>
struct is_volatile_helper_1<const volatile T *> : public true_t {};

template <typename T>
struct is_volatile_helper_2 : public is_volatile_helper_1<T *> {};
template <typename T>
struct is_volatile_helper_2<T &> : public false_t {};  // Note here that T is volatile, not the reference to T. So is_volatile is false.

template <typename T>
concept is_volatile = is_volatile_helper_2<T>::value;

// Checks if T is a reference (includes reference to function types)
template <typename T>
struct is_reference_helper : public false_t {};
template <typename T>
struct is_reference_helper<T &> : public true_t {};

template <typename T>
concept is_reference = is_reference_helper<T>::value;

// Checks if T is a r-value reference (includes reference to function types)
template <typename T>
struct is_rvalue_reference_helper : public false_t {};
template <typename T>
struct is_rvalue_reference_helper<T &&> : public true_t {};

template <typename T>
concept is_rvalue_reference = is_rvalue_reference_helper<T>::value;

// Checks if T is a function type (doesn't include member functions)
template <typename>
struct is_function_helper : public false_t {};

template <typename R, typename... Args>
struct is_function_helper<R(Args...)> : public true_t {};

template <typename R, typename... Args>
struct is_function_helper<R(Args..., ...)> : public true_t {};

template <typename T>
concept is_function = is_function_helper<T>::value;

template <typename T>
struct is_void_helper : public false_t {};

template <>
struct is_void_helper<void> : public true_t {};

template <typename T>
concept is_void = is_void_helper<remove_cv_t<T>>::value;

template <typename T>
concept is_null = is_same<remove_cv_t<T>, decltype(null)>;

// The remove_const transformation trait removes top-level const
// qualification (if any) from the type to which it is applied.
// For a given type T, remove_const<T const>::type is equivalent to the type T.
// For example, remove_const<char*>::type is equivalent to char* while
// remove_const<const char*>::type is equivalent to const char*.
// In the latter case, the const qualifier modifies char, not *, and is
// therefore not at the top level.

template <typename T>
struct remove_const {
    using type = T;
};

template <typename T>
struct remove_const<const T> {
    using type = T;
};

template <typename T>
struct remove_const<const T[]> {
    using type = T[];
};

template <typename T, int N>
struct remove_const<const T[N]> {
    using type = T[N];
};

template <typename T>
using remove_const_t = typename remove_const<T>::type;

// The remove_volatile transformation trait removes top-level const
// qualification (if any) from the type to which it is applied.
// For a given type T, the type remove_volatile <T volatile>::T is equivalent
// to the type T. For example, remove_volatile <char* volatile>::type is
// equivalent to char* while remove_volatile <volatile char*>::type is
// equivalent to volatile char*. In the latter case, the volatile qualifier
// modifies char, not *, and is therefore not at the top level.

template <typename T>
struct remove_volatile {
    using type = T;
};

template <typename T>
struct remove_volatile<volatile T> {
    using type = T;
};

template <typename T>
struct remove_volatile<volatile T[]> {
    using type = T[];
};

template <typename T, int N>
struct remove_volatile<volatile T[N]> {
    using type = T[N];
};

template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

// Remove top-level const/volatile
template <typename T>
struct remove_cv {
    using type = remove_volatile_t<remove_const_t<T>>;
};

template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

// The remove_reference transformation trait removes top-level of
// indirection by reference (if any) from the type to which it is applied.
// For a given type T, remove_reference<T&>::type is equivalent to T.
template <typename T>
struct remove_reference {
    using type = T;
};
template <typename T>
struct remove_reference<T &> {
    using type = T;
};

template <typename T>
struct remove_reference<T &&> {
    using type = T;
};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

// The remove_cvref transformation trait removes top-level const and/or volatile
// qualification (if any) from the reference type to which it is applied. For a given type T&,
// remove_cvref<T& const volatile>::type is equivalent to T. For example,
// remove_cv<int& volatile>::type is equivalent to int.
template <typename T>
struct remove_cvref {
    using type = remove_volatile_t<remove_const_t<remove_reference_t<T>>>;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

// Add const to a type
template <typename T, bool = is_const<T> || is_reference<T> || is_function<T>>
struct add_const_helper {
    using type = T;
};

template <typename T>
struct add_const_helper<T, false> {
    using type = const T;
};

template <typename T>
struct add_const {
    using type = typename add_const_helper<T>::type;
};

template <typename T>
using add_const_t = typename add_const<T>::type;

// Add volatile to a type
template <typename T, bool = is_volatile<T> || is_reference<T> || is_function<T>>
struct add_volatile_helper {
    using type = T;
};

template <typename T>
struct add_volatile_helper<T, false> {
    using type = volatile T;
};

template <typename T>
struct add_volatile {
    using type = typename add_volatile_helper<T>::type;
};

template <typename T>
using add_volatile_t = typename add_volatile<T>::type;

// The add_reference transformation trait adds a level of indirection
// by reference to the type to which it is applied.
//
// Rules (8.3.2 p6):
//      void + &  -> void
//      T    + &  -> T&
//      T&   + &  -> T&
//      T&&  + &  -> T&
template <typename T>
struct add_reference_impl {
    using type = T &;
};
template <typename T>
struct add_reference_impl<T &> {
    using type = T &;
};
template <>
struct add_reference_impl<void> {
    using type = void;
};
#if COMPILER != MSVC
template <typename T>
struct add_reference_impl<T[0]> {
    using type = T;
};
#endif

template <typename T>
struct add_reference {
    using type = typename add_reference_impl<T>::type;
};

template <typename T>
using add_reference_t = typename add_reference<T>::type;

// Rules (8.3.2 p6):
//      void + &&  -> void
//      T    + &&  -> T&&
//      T&   + &&  -> T&
//      T&&  + &&  -> T&&
template <typename T>
struct add_rvalue_reference {
    using type = T &&;
};
template <typename T>
struct add_rvalue_reference<T &> {
    using type = T &;
};
template <>
struct add_rvalue_reference<void> {
    using type = void;
};
template <>
struct add_rvalue_reference<const void> {
    using type = const void;
};
template <>
struct add_rvalue_reference<volatile void> {
    using type = volatile void;
};
template <>
struct add_rvalue_reference<const volatile void> {
    using type = const volatile void;
};

template <typename T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

// Converts any type T to a reference type, making it possible to use member functions in
// decltype expressions without specifying constructors. It has no use outside decltype expressions.
// By design there is no implementation, as it's never executed but rather is used only in decltype expressions.
// The C++11 Standard section 20.2.4 states that we must declare this.
// http://en.cppreference.com/w/cpp/utility/declval
template <typename T>
typename add_rvalue_reference<T>::type declval() noexcept;

//
// Concept satisfied if T is one of the following types: bool, char, wchar_t, short, int, long long (and unsigned variants)
//
template <typename T>
struct is_integral_helper : public false_t {};

template <>
struct is_integral_helper<unsigned char> : public true_t {};
template <>
struct is_integral_helper<unsigned short> : public true_t {};
template <>
struct is_integral_helper<unsigned int> : public true_t {};
template <>
struct is_integral_helper<unsigned long> : public true_t {};
template <>
struct is_integral_helper<unsigned long long> : public true_t {};

template <>
struct is_integral_helper<signed char> : public true_t {};
template <>
struct is_integral_helper<signed short> : public true_t {};
template <>
struct is_integral_helper<signed int> : public true_t {};
template <>
struct is_integral_helper<signed long> : public true_t {};
template <>
struct is_integral_helper<signed long long> : public true_t {};

template <>
struct is_integral_helper<bool> : public true_t {};
template <>
struct is_integral_helper<char> : public true_t {};
template <>
struct is_integral_helper<char8_t> : public true_t {};
template <>
struct is_integral_helper<char16_t> : public true_t {};
template <>
struct is_integral_helper<char32_t> : public true_t {};

#if defined _NATIVE_WCHAR_T_DEFINED
template <>
struct is_integral_helper<wchar_t> : public true_t {};
#endif

// Use this macro to declare your custom type as an integral
#define DECLARE_INTEGRAL(T)                          \
    LSTD_BEGIN_NAMESPACE                             \
    template <>                                      \
    struct is_integral_helper<T> : public true_t {}; \
    LSTD_END_NAMESPACE

template <typename T>
concept is_integral = is_integral_helper<remove_cv_t<T>>::value;

template <typename T>
concept is_signed_integral = (is_integral<T> && T(-1) < T(0));

template <typename T>
concept is_unsigned_integral = is_integral<T> && !is_signed_integral<T>;

//
// Concept satisfied if T is float, double
//
template <typename T>
struct is_floating_point_helper : public false_t {};

template <>
struct is_floating_point_helper<float> : public true_t {};
template <>
struct is_floating_point_helper<double> : public true_t {};

// Use this macro to declare your custom type as a floating point
#define DECLARE_FLOATING_POINT(T)                          \
    LSTD_BEGIN_NAMESPACE                                   \
    template <>                                            \
    struct is_floating_point_helper<T> : public true_t {}; \
    LSTD_END_NAMESPACE

template <typename T>
concept is_floating_point = is_floating_point_helper<remove_cv_t<T>>::value;

//
// An arithmetic type is an integral type or a floating point type
//
template <typename T>
concept is_arithmetic = is_integral<T> || is_floating_point<T>;

//
// A fundamental type is void, nullptr_t, or any of the arithmetic types
//
template <typename T>
concept is_fundamental = is_void<T> || is_null<T> || is_arithmetic<T>;

template <typename T>
concept is_union = __is_union(remove_cv<T>);

template <typename T>
concept is_class = __is_class(remove_cv<T>);

template <typename T>
concept is_enum = __is_enum(remove_cv_t<T>);

//
// An object considered to be any type that is not a function a reference or void
//
template <typename T>
concept is_object = !is_reference_v<T> && !is_void_v<T> && !is_function_v<T>;

//
// True if T is a pointer to a member function AND NOT a member object
//
template <typename T>
struct is_member_function_pointer_helper : false_t {};

template <typename T, typename U>
struct is_member_function_pointer_helper<T U::*> : is_function_helper<T> {};

template <typename T>
concept is_member_function_pointer = is_member_function_pointer_helper<remove_cv_t<T>>::value;

//
// True if T is a pointer to a member object OR a member function
//
template <typename T>
struct is_member_pointer_helper : false_t {};

template <typename T, typename U>
struct is_member_pointer_helper<T U::*> : true_t {};

template <typename T>
concept is_member_pointer = is_member_pointer_helper<remove_cv_t<T>>::value;

//
// True if T is a pointer to a member object AND NOT a member function
//
template <typename T>
concept is_member_object_pointer = is_member_pointer<T> && !is_member_function_pointer<T>;

// Tests whether T is a pointer to an object, to a function, but not to member objects/functions (use the tests above for that)
//
template <typename T>
struct is_pointer_helper : false_t {};

template <typename T>
struct is_pointer_helper<T *> : true_t {};

template <typename T>
concept is_pointer = is_pointer_helper<remove_cv_t<T>>::value;

//
// A scalar is an integer type, a floating point type, an enum type, a pointer or a member function pointer or a null pointer type.
//
template <typename T>
concept is_scalar = is_arithmetic<T> || is_enum<T> || is_pointer<T> || is_member_pointer<T> || is_null<T>;

template <typename From, typename To>
concept is_convertible = __is_convertible_to(From, To);

template <typename T, typename... Args>
concept is_constructible = __is_constructible(T, Args...);

//
// Gets the underlying type of an enum
//
template <typename T>
struct underlying_type_helper { using type = __underlying_type(T); };

template <typename T>
using underlying_type_t = typename underlying_type_helper<T>::type;

//
// Stuff to do with arrays:
//

// Rank returns the number of dimensions on af array
template <typename T>
struct rank_helper : public integral_constant<int, 0> {};

template <typename T>
struct rank_helper<T[]> : public integral_constant<int, rank_helper<T>::value + 1> {};

template <typename T, int N>
struct rank_helper<T[N]> : public integral_constant<int, rank_helper<T>::value + 1> {};

template <typename T>
constexpr int rank = rank<T>::value;

// An integral type representing the number of elements in the Ith dimension of array type T.
//
// For a given array type T[N], extent<T[N]>::value == N.
// For a given multi-dimensional array type T[M][N], extent<T[M][N], 0>::value == N.
// For a given multi-dimensional array type T[M][N], extent<T[M][N], 1>::value == M.
// For a given array type T and a given dimension I where I >= rank<T>::value, extent<T, I>::value == 0.
// For a given array type of unknown extent T[], extent<T[], 0>::value == 0.
// For a given non-array type T and an arbitrary dimension I, extent<T, I>::value == 0.
template <typename T, int N>
struct extent_helper : public integral_constant<int, 0> {};

template <typename T, int I>
struct extent_helper<T[I], 0> : public integral_constant<int, I> {};

template <typename T, int N, int I>
struct extent_helper<T[I], N> : public extent_helper<T, N - 1> {};

template <typename T, int N>
struct extent_helper<T[], N> : public extent_helper<T, N - 1> {};

template <typename T, int N = 0>
constexpr int extent_v = extent_helper<T, N>::value;

template <typename T>
struct is_array_helper : public false_t {};

template <typename T>
struct is_array_helper<T[]> : public true_t {};

template <typename T, int N>
struct is_array_helper<T[N]> : public true_t {};

template <typename T>
concept is_array = is_array_helper<T>::value;

// Not part of the C++ Standard.
template <typename T>
struct is_array_of_known_bounds : public integral_constant<bool, extent_v<T> != 0> {};

template <typename T>
constexpr bool is_array_of_known_bounds_v = is_array_of_known_bounds<T>::value;

// Not part of the C++ Standard.
template <typename T>
struct is_array_of_unknown_bounds : public integral_constant<bool, is_array<T> && (extent_v<T> == 0)> {};

template <typename T>
constexpr bool is_array_of_unknown_bounds_v = is_array_of_unknown_bounds<T>::value;

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

// Determines the common type among all types T..., that is the type all T... can be implicitly converted to.
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

// Are these useful?
//
// template <typename... Types>
// using common_comparison_category_t = select_t<(comparison_category_of<Types...> & Comparison_Category_None) != 0, void,
//                                               select_t<(comparison_category_of<Types...> & Comparison_Category_Partial) != 0, partial_ordering,
//                                                        select_t<(comparison_category_of<Types...> & Comparison_Category_Weak) != 0, weak_ordering,
//                                                                 strong_ordering>>>;
//
// template <typename... Types>
// struct common_comparison_category {
//     using type = common_comparison_category_t<Types...>;
// };
}  // namespace types

//
// Some common functions:
//

constexpr bool sign_bit(types::is_signed_integral auto x) { return x < 0; }
constexpr bool sign_bit(types::is_unsigned_integral auto) { return false; }

constexpr bool sign_bit(f32 x) { return ieee754_f32{x}.ieee.S; }
constexpr bool sign_bit(f64 x) { return ieee754_f64{x}.ieee.S; }

// Returns -1 if x is negative, 1 otherwise
constexpr s32 sign_no_zero(types::is_scalar auto x) { return sign_bit(x) ? -1 : 1; }

// Returns -1 if x is negative, 1 if positive, 0 otherwise
constexpr s32 sign(types::is_scalar auto x) {
    if (x == decltype(x)(0)) return 0;
    return sign_no_zero(x);
}

template <types::is_floating_point T>
constexpr T copy_sign(T x, T y) {
    if constexpr (sizeof(x) == sizeof(f32)) {
        ieee754_f32 formatx = {x}, formaty = {y};
        formatx.ieee.S = formaty.ieee.S;
        return formatx.F;
    } else {
        ieee754_f64 formatx = {x}, formaty = {y};
        formatx.ieee.S = formaty.ieee.S;
        return formatx.F;
    }
}

constexpr bool is_nan(types::is_floating_point auto x) {
    if constexpr (sizeof(x) == sizeof(f32)) {
        ieee754_f32 format = {x};
        return format.ieee.E == 255 && format.ieee.M != 0;
    } else {
        ieee754_f64 format = {x};
        return format.ieee.E == 2048 && (format.ieee.M0 != 0 || format.ieee.M1 != 0);
    }
}

constexpr bool is_signaling_nan(types::is_floating_point auto x) {
    if constexpr (sizeof(x) == sizeof(f32))
        return is_nan(x) && ieee754_f32{x}.ieee_nan.N == 0;
    else
        return is_nan(x) && ieee754_f64{x}.ieee_nan.N == 0;
}

constexpr bool is_infinite(types::is_floating_point auto x) {
    if constexpr (sizeof(x) == sizeof(f32)) {
        ieee754_f32 format = {x};
        return format.ieee.E == 255 && format.ieee.M == 0;
    } else {
        ieee754_f64 format = {x};
        return format.ieee.E == 2048 && format.ieee.M0 == 0 && format.ieee.M1 == 0;
    }
}

constexpr bool is_finite(types::is_floating_point auto x) {
    if constexpr (sizeof(x) == sizeof(f32))
        return ieee754_f32{x}.ieee.E != 255;
    else
        return ieee754_f64{x}.ieee.E != 2048;
}

template <typename T, typename U>
concept are_same_signage = (types::is_signed_integral<T> && types::is_signed_integral<U>) || (types::is_unsigned_integral<T> && types::is_unsigned_integral<U>);

template <typename T, typename U>
requires(types::is_scalar<T> &&types::is_scalar<U>) constexpr T cast_numeric_safe(U y) {
    if constexpr (types::is_floating_point<T>) {
        static_assert((types::is_integral<U> || sizeof(T) >= sizeof(U)), "T is a float. U must be a float of the same size or smaller, or an integer. Otherwise information may be lost when casting.");
    } else if constexpr (types::is_integral<T> && types::is_integral<U>) {
        static_assert(sizeof(T) > sizeof(U) || (sizeof(T) == sizeof(U) && are_same_signage<T, U>), "Both T and U are integers. T must be larger than U, or if they have the same size, they must have the same signage. Otherwise information may be lost when casting.");
    } else {
        static_assert(false, "T was an integer, but U was a floating point. Information may be lost when casting.");
    }
    return (T) y;
}

constexpr auto min_(auto x, auto y) {
    auto y_casted = cast_numeric_safe<decltype(x)>(y);
    if constexpr (types::is_floating_point<decltype(x)>) {
        if (is_nan(x) || is_nan(y_casted)) return x + y_casted;
    }
    return x < y_casted ? x : y_casted;
}

constexpr auto max_(auto x, auto y) {
    auto y_casted = cast_numeric_safe<decltype(x)>(y);
    if constexpr (types::is_floating_point<decltype(x)>) {
        if (is_nan(x) || is_nan(y_casted)) return x + y_casted;
    }
    return x > y_casted ? x : y_casted;
}

template <types::is_scalar... Args>
constexpr auto min(types::is_scalar auto x, Args... rest) {
    auto result = x;
    ((void) (result = min_(result, rest)), ...);
    return result;
}

template <types::is_scalar... Args>
constexpr auto max(types::is_scalar auto x, Args... rest) {
    auto result = x;
    ((void) (result = max_(result, rest)), ...);
    return result;
}

// Returns lower if x < lower, return upper if x > upper, returns x otherwise
constexpr always_inline auto clamp(auto x, auto lower, auto upper) { return max(lower, min(upper, x)); }

// Checks if x is a power of 2
constexpr bool is_pow_of_2(types::is_integral auto x) { return (x & (x - 1)) == 0; }

// Returns the smallest power of 2 bigger or equal to x.
template <types::is_integral T>
constexpr auto ceil_pow_of_2(T x) {
    if (x <= 1) return T(1);

    T power = 2;
    --x;
    while (x >>= 1) power <<= 1;
    return power;
}

// Returns 10 ** exp at compile-time. Uses recursion.
template <typename T>
constexpr T const_exp10(s32 exp) { return exp == 0 ? T(1) : T(10) * const_exp10<T>(exp - 1); }

constexpr auto abs(types::is_scalar auto x) {
    if constexpr (types::is_floating_point<decltype(x)>) {
        if constexpr (sizeof(x) == sizeof(f32)) {
            ieee754_f32 u = {x};
            u.ieee.S = 0;
            return u.F;
        } else {
            ieee754_f64 u = {x};
            u.ieee.S = 0;
            return u.F;
        }
    } else {
        if constexpr (types::is_unsigned_integral<decltype(x)>) {
            return x;  // Unsigned integrals are always positive
        } else {
            return x < 0 ? -x : x;
        }
    }
}

LSTD_END_NAMESPACE
