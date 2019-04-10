#pragma once

#include "namespace.h"
#include "platform.h"

/// Provides definitions for common types as well as most stuff from <type_traits> from the std

// The following integral types are defined: s8, s16, s32, s64 (and corresponding unsigned types: u8, u16, u32, u64)
//		as well as: f32 (float), f64 (double), byte (char), null (nullptr), npos ((size_t) -1)
// Platform dependent types: ptr_t (signed, used for pointer difference)
//				uptr_t, size_t (32 or 64 bits depending on CPU architecture)
//
// Min/max values are also defined (S8_MIN, S8_MAX, etc...)
//
// What's missing from std:
// - conditional (type_select is equivalent)
// - is_trivially_default_constructible (is_trivially_constructible is equivalent)
// - is_default_constructible (is_constructible is equivalent)
// - is_explicitly_convertible (is_convertible is equivalent)
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

LSTD_BEGIN_NAMESPACE

// Used internally to denote a special template argument that means it's an unused argument.
struct unused {};

// This is essentially a utility base struct for defining properties as both struct constants and as types.
template <typename T, T Value>
struct integral_constant {
    static constexpr T value = Value;

    using value_t = T;
    using type = integral_constant<T, Value>;

    constexpr operator value_t() const { return value; }
    constexpr value_t operator()() const { return value; }
};

using true_t = integral_constant<bool, true>;
using false_t = integral_constant<bool, false>;

// These are used as a utility to differentiate between two things.
// sizeof(yes_t) == 1
using yes_t = char;

// sizeof(no_t)  != 1
struct no_t {
    char Padding[8];
};

// Used as a type which constructs from anything.
struct argument_sink {
    template <typename... Args>
    argument_sink(Args &&...) {}
};

// This is used to declare a type from one of two type options.
// The result is based on the condition type.
//
// Example usage:
//    using chosen_t = type_select_t<is_integral_v<SomeType>, ChoiceAType, ChoiceBType>;
//
template <bool Condition, typename ConditionIsTrueType, typename ConditionIsFalseType>
struct type_select {
    using type = ConditionIsTrueType;
};

template <typename ConditionIsTrueType, typename ConditionIsFalseType>
struct type_select<false, ConditionIsTrueType, ConditionIsFalseType> {
    using type = ConditionIsFalseType;
};

template <bool Condition, typename ConditionIsTrueType, typename ConditionIsFalseType>
using type_select_t = typename type_select<Condition, ConditionIsTrueType, ConditionIsFalseType>::type;

// Similar to type_select but unilaterally selects the first type.
template <typename T, typename = unused, typename = unused>
struct first_type_select {
    using type = T;
};

template <typename T, typename = unused, typename = unused>
using first_type_select_t = typename first_type_select<T>::type;

// This is a utility struct for creating composite type traits.
template <bool B1, bool B2, bool B3 = false, bool B4 = false, bool B5 = false>
struct type_or;

template <bool B1, bool B2, bool B3, bool B4, bool B5>
struct type_or {
    static constexpr auto value = true;
};

template <>
struct type_or<false, false, false, false, false> {
    static constexpr auto value = false;
};

// This is a utility struct for creating composite type traits.
template <bool B1, bool B2, bool B3 = true, bool B4 = true, bool B5 = true>
struct type_and;

template <bool B1, bool B2, bool B3, bool B4, bool B5>
struct type_and {
    static constexpr auto value = false;
};

template <>
struct type_and<true, true, true, true, true> {
    static constexpr auto value = true;
};

// This is a utility struct for creating composite type traits.
template <int B1, int B2>
struct type_equal {
    static constexpr auto value = (B1 == B2);
};

// This is a utility struct for creating composite type traits.
template <int B1, int B2>
struct type_not_equal {
    static constexpr auto value = (B1 != B2);
};

// This is a utility struct for creating composite type traits.
template <bool B>
struct type_not {
    static constexpr auto value = true;
};

template <>
struct type_not<true> {
    static constexpr auto value = false;
};

// template <bool B, typename T = void> struct enable_if;
template <bool B, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
    using type = T;
};

template <bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

// template <bool B, typename T = void> struct disable_if;
template <bool B, typename T = void>
struct disable_if {};

template <typename T>
struct disable_if<false, T> {
    using type = T;
};

template <bool B, typename T = void>
using disable_if_t = typename disable_if<B, T>::type;

template <typename...>
struct disjunction : false_t {};

template <typename B>
struct disjunction<B> : B {};

template <typename B, typename... Bn>
struct disjunction<B, Bn...> : type_select_t<bool(B::value), B, disjunction<Bn...>> {};

template <typename... B>
inline bool disjunction_v = disjunction<B...>::value;

template <typename B>
struct negation : integral_constant<bool, !bool(B::value)> {};

template <typename B>
inline bool negation_v = negation<B>::value;

// The purpose of this is typically to deal with non-deduced template contexts
template <typename T>
struct type_identity {
    using type = T;
};

template <typename T>
using type_identity_t = typename type_identity<T>::type;

// Checks if two types are the same
template <typename T, typename U>
struct is_same : public false_t {};

template <typename T>
struct is_same<T, T> : public true_t {};

template <typename T, typename U>
constexpr bool is_same_v = is_same<T, U>::value;

// Checks if T has const-qualification
template <typename T>
struct is_const_value : public false_t {};
template <typename T>
struct is_const_value<volatile T *> : public true_t {};
template <typename T>
struct is_const_value<const volatile T *> : public true_t {};

template <typename T>
struct is_const : public is_const_value<T *> {};
template <typename T>
struct is_const<T &> : public false_t {};  // Note here that T is const, not the reference to T. So is_const is false.

template <typename T>
constexpr bool is_const_v = is_const<T>::value;

// Checks if T has volatile-qualification
template <typename T>
struct is_volatile_value : public false_t {};
template <typename T>
struct is_volatile_value<const T *> : public true_t {};
template <typename T>
struct is_volatile_value<const volatile T *> : public true_t {};

template <typename T>
struct is_volatile : public is_volatile_value<T *> {};
template <typename T>
struct is_volatile<T &> : public false_t {
};  // Note here that T is volatile, not the reference to T. So is_volatile is false.

template <typename T>
constexpr bool is_volatile_v = is_volatile<T>::value;

// Checks if T is a reference (includes reference to function types)
template <typename T>
struct is_reference : public false_t {};
template <typename T>
struct is_reference<T &> : public true_t {};

template <typename T>
constexpr bool is_reference_v = is_reference<T>::value;

// Checks if T is a function type (doesn't include member functions)
template <typename>
struct is_function : public false_t {};

#if BITS == 32 && COMPILER == MSVC
// __cdecl specialization
template <typename ReturnValue, typename... ArgPack>
struct is_function<ReturnValue __cdecl(ArgPack...)> : public true_t {};

template <typename ReturnValue, typename... ArgPack>
struct is_function<ReturnValue __cdecl(ArgPack..., ...)> : public true_t {};

// __stdcall specialization
template <typename ReturnValue, typename... ArgPack>
struct is_function<ReturnValue __stdcall(ArgPack...)> : public true_t {};

// When functions use a variable number of arguments, it is the caller that cleans the stack (cf. cdecl).
//
// template <typename ReturnValue, typename... ArgPack>
// struct is_function<ReturnValue __stdcall (ArgPack..., ...)>
//     : public true_t {};
#else
template <typename ReturnValue, typename... ArgPack>
struct is_function<ReturnValue(ArgPack...)> : public true_t {};

template <typename ReturnValue, typename... ArgPack>
struct is_function<ReturnValue(ArgPack..., ...)> : public true_t {};
#endif

template <typename T>
constexpr bool is_function_v = is_function<T>::value;

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
template <typename T, size_t N>
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
template <typename T, size_t N>
struct remove_volatile<volatile T[N]> {
    using type = T[N];
};

template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

// Remove top-level const/volatile
template <typename T>
struct remove_cv {
    using type = typename remove_volatile_t<typename remove_const_t<T>>;
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
    using type = typename remove_volatile_t<typename remove_const_t<typename remove_reference_t<T>>>;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

// Add const to a type
template <typename T, bool = is_const_v<T> || is_reference_v<T> || is_function_v<T>>
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
template <typename T, bool = is_volatile_v<T> || is_reference_v<T> || is_function_v<T>>
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

// Add const and volatile
template <typename T>
struct add_cv {
    using type = typename add_const_t<typename add_volatile_t<T>>;
};

template <typename T>
using add_cv_t = typename add_cv<T>::type;

// The add_reference transformation trait adds a level of indirection
// by reference to the type to which it is applied. For a given type T,
// add_reference<T>::type is equivalent to T& if is_reference<T>::value == false,
// and T otherwise.
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
//      void + &  -> void
//      T    + &  -> T&
//      T&   + &  -> T&
//      T&&  + &  -> T&
template <typename T>
struct add_lvalue_reference {
    using type = T &;
};
template <typename T>
struct add_lvalue_reference<T &> {
    using type = T &;
};
template <>
struct add_lvalue_reference<void> {
    using type = void;
};
template <>
struct add_lvalue_reference<const void> {
    using type = const void;
};
template <>
struct add_lvalue_reference<volatile void> {
    using type = volatile void;
};
template <>
struct add_lvalue_reference<const volatile void> {
    using type = const volatile void;
};

template <typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

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

// These are primarily useful in templated code for meta programming.
// Currently we are limited to size_t, as C++ doesn't allow integral
// template parameters to be generic. We can expand the supported types
// to include additional integers if needed.
template <size_t I0, size_t... in>
struct static_min;

template <size_t I0>
struct static_min<I0> {
    static constexpr auto value = I0;
};

template <size_t I0, size_t I1, size_t... in>
struct static_min<I0, I1, in...> {
    static constexpr auto value = ((I0 <= I1) ? static_min<I0, in...>::value : static_min<I1, in...>::value);
};

template <size_t I0, size_t... in>
inline size_t static_min_v = static_min<I0, in...>::value;

template <size_t I0, size_t... in>
struct static_max;

template <size_t I0>
struct static_max<I0> {
    static constexpr auto value = I0;
};

template <size_t I0, size_t I1, size_t... in>
struct static_max<I0, I1, in...> {
    static constexpr auto value = ((I0 >= I1) ? static_max<I0, in...>::value : static_max<I1, in...>::value);
};

template <size_t I0, size_t... in>
inline size_t static_max_v = static_max<I0, in...>::value;

//
// Fundamental types:
//
using s8 = char;
using s16 = short;
using s32 = int;
using s64 = long long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned;
using u64 = unsigned long long;

#if BITS == 64
#define S64_C(c) c##L
#define U64_C(c) c##UL
#else
#define S64_C(c) c##LL
#define U64_C(c) c##ULL
#endif

#define S8_MIN (-128)
#define S16_MIN (-32767 - 1)
#define S32_MIN (-2147483647 - 1)
#define S64_MIN (-S64_C(9223372036854775807L) - 1)

#define S8_MAX (127)
#define S16_MAX (32767)
#define S32_MAX (2147483647)
#define S64_MAX (S64_C(9223372036854775807))

#define U8_MAX (255)
#define U16_MAX (65535)
#define U32_MAX (4294967295U)
#define U64_MAX (U64_C(18446744073709551615))

#undef S64_C
#undef U64_C

using byte = char;

#define BYTE_MIN S8_MIN
#define BYTE_MAX S8_MAX

using f32 = float;
using f64 = double;

#define F64_DECIMAL_DIG 17                    // # of decimal digits of rounding precision
#define F64_DIG 15                            // # of decimal digits of precision
#define F64_EPSILON 2.2204460492503131e-016   // smallest such that 1.0+F64_EPSILON != 1.0
#define F64_HAS_SUBNORM 1                     // type does support subnormal numbers
#define F64_MANT_DIG 53                       // # of bits in mantissa
#define F64_MAX 1.7976931348623158e+308       // max value
#define F64_MAX_10_EXP 308                    // max decimal exponent
#define F64_MAX_EXP 1024                      // max binary exponent
#define F64_MIN 2.2250738585072014e-308       // min positive value
#define F64_MIN_10_EXP (-307)                 // min decimal exponent
#define F64_MIN_EXP (-1021)                   // min binary exponent
#define F64_RADIX 2                           // exponent radix
#define F64_TRUE_MIN 4.9406564584124654e-324  // min positive value

#define F32_DECIMAL_DIG 9             // # of decimal digits of rounding precision
#define F32_DIG 6                     // # of decimal digits of precision
#define F32_EPSILON 1.192092896e-07F  // smallest such that 1.0+F32_EPSILON != 1.0
#define F32_HAS_SUBNORM 1             // type does support subnormal numbers
#define F32_GUARD 0
#define F32_MANT_DIG 24           // # of bits in mantissa
#define F32_MAX 3.402823466e+38F  // max value
#define F32_MAX_10_EXP 38         // max decimal exponent
#define F32_MAX_EXP 128           // max binary exponent
#define F32_MIN 1.175494351e-38F  // min normalized positive value
#define F32_MIN_10_EXP (-37)      // min decimal exponent
#define F32_MIN_EXP (-125)        // min binary exponent
#define F32_NORMALIZE 0
#define F32_RADIX 2                    // exponent radix
#define F32_TRUE_MIN 1.401298464e-45F  // min positive value

#define LONG_F64_DIG F64_DIG                  // # of decimal digits of precision
#define LONG_F64_EPSILON F64_EPSILON          // smallest such that 1.0+LONG_F64_EPSILON != 1.0
#define LONG_F64_HAS_SUBNORM F64_HAS_SUBNORM  // type does support subnormal numbers
#define LONG_F64_MANT_DIG F64_MANT_DIG        // # of bits in mantissa
#define LONG_F64_MAX F64_MAX                  // max value
#define LONG_F64_MAX_10_EXP F64_MAX_10_EXP    // max decimal exponent
#define LONG_F64_MAX_EXP F64_MAX_EXP          // max binary exponent
#define LONG_F64_MIN F64_MIN                  // min normalized positive value
#define LONG_F64_MIN_10_EXP F64_MIN_10_EXP    // min decimal exponent
#define LONG_F64_MIN_EXP F64_MIN_EXP          // min binary exponent
#define LONG_F64_RADIX F64_RADIX              // exponent radix
#define LONG_F64_TRUE_MIN F64_TRUE_MIN        // min positive value

#if !defined DECIMAL_DIG
#define DECIMAL_DIG F64_DECIMAL_DIG
#endif

// Personal preference
// I prefer null over nullptr but they are exactly the same
constexpr auto null = nullptr;

// Used for search results (not found)
// Inspired by C++'s std
constexpr auto npos = (size_t) -1;

// uptr_t is the minimum size unsigned integer that can hold the address of any byte in RAM
// ptr_t is used to represent the difference of addresses (pointers)
#if BITS == 64
using uptr_t = u64;
using ptr_t = s64;

#define PTR_MIN S64_MIN
#define PTR_MAX S64_MAX
#define UPTR_MAX U64_MAX
#else
using ptr_t = s32;
using uptr_t = u32;

#define PTR_MIN S32_MIN
#define PTR_MAX S32_MAX
#define UPTR_MAX U32_MAX
#endif

// size_t is used to represent any size (or index) in bytes
using size_t = uptr_t;
#if !defined SIZE_MAX
#define SIZE_MAX UPTR_MAX
#endif

#define WCHAR_MIN 0x0000
#define WCHAR_MAX 0xffff

template <typename T>
struct is_void : public false_t {};

template <>
struct is_void<void> : public true_t {};
template <>
struct is_void<void const> : public true_t {};
template <>
struct is_void<void volatile> : public true_t {};
template <>
struct is_void<void const volatile> : public true_t {};

template <typename T>
constexpr bool is_void_v = is_void<T>::value;

// Utility which identifies if any of the given template arguments is void.
template <typename... Args>
struct has_void_arg;

template <>
struct has_void_arg<> : public false_t {};

template <typename A0, typename... Args>
struct has_void_arg<A0, Args...> {
    static constexpr auto value = (is_void<A0>::value || has_void_arg<Args...>::value);
};

template <typename... Args>
constexpr bool has_void_arg_v = has_void_arg<Args...>::value;

template <typename T>
struct is_null_pointer : public is_same<typename remove_cv_t<T>, decltype(null)> {};

template <typename T>
constexpr bool is_null_pointer_v = is_null_pointer<T>::value;

// true if T is one of the following types: bool, char, wchar_t, short, int, long long (and unsigned variants)
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
struct is_integral_helper<char16_t> : public true_t {};
template <>
struct is_integral_helper<char32_t> : public true_t {};

#if defined _NATIVE_WCHAR_T_DEFINED
template <>
struct is_integral_helper<wchar_t> : public true_t {};
#endif

template <typename T>
struct is_integral : public is_integral_helper<typename remove_cv_t<T>> {};

// Use this macro to declare your custom type as an integral
#define DECLARE_INTEGRAL(T)                                  \
    LSTD_BEGIN_NAMESPACE                                     \
    template <>                                              \
    struct is_integral<T> : public true_t {};                \
    template <>                                              \
    struct is_integral<const T> : public true_t {};          \
    template <>                                              \
    struct is_integral<volatile T> : public true_t {};       \
    template <>                                              \
    struct is_integral<const volatile T> : public true_t {}; \
    LSTD_END_NAMESPACE

template <typename T>
constexpr bool is_integral_v = is_integral<T>::value;

// true if T is one of the following types: float, double, long double
template <typename T>
struct is_floating_point_helper : public false_t {};

template <>
struct is_floating_point_helper<float> : public true_t {};
template <>
struct is_floating_point_helper<double> : public true_t {};
template <>
struct is_floating_point_helper<long double> : public true_t {};

template <typename T>
struct is_floating_point : public is_floating_point_helper<typename remove_cv_t<T>> {};

// Use this macro to declare your custom type as a floating point
#define DECLARE_FLOATING_POINT(T)                                  \
    LSTD_BEGIN_NAMESPACE                                           \
    template <>                                                    \
    struct is_floating_point<T> : public true_t {};                \
    template <>                                                    \
    struct is_floating_point<const T> : public true_t {};          \
    template <>                                                    \
    struct is_floating_point<volatile T> : public true_t {};       \
    template <>                                                    \
    struct is_floating_point<const volatile T> : public true_t {}; \
    LSTD_END_NAMESPACE

template <typename T>
constexpr bool is_floating_point_v = is_floating_point<T>::value;

// is_floating_point<T> || is_integral<T>
template <typename T>
struct is_arithmetic : public integral_constant<bool, is_integral_v<T> || is_floating_point_v<T>> {};

template <typename T>
constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

// is_floating_point<T> || s_integral<T> || is_void<T> || is_null_pointer<T>
template <typename T>
struct is_fundamental : public integral_constant<bool, is_void_v<T> || is_integral_v<T> || is_floating_point_v<T> ||
                                                           is_null_pointer_v<T>> {};

template <typename T>
constexpr bool is_fundamental_v = is_fundamental<T>::value;

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
struct remove_pointer<T *const> {
    using type = T;
};
template <typename T>
struct remove_pointer<T *volatile> {
    using type = T;
};
template <typename T>
struct remove_pointer<T *const volatile> {
    using type = T;
};

template <typename T>
using remove_pointer_t = typename remove_pointer<T>::type;

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
template <typename T, size_t N>
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
template <typename T, size_t N>
struct remove_all_extents<T[N]> {
    using type = typename remove_all_extents<T>::type;
};
template <typename T>
struct remove_all_extents<T[]> {
    using type = typename remove_all_extents<T>::type;
};

template <typename T>
using remove_all_extents_t = typename remove_all_extents<T>::type;

// The aligned_storage transformation trait provides a type that is
// suitably aligned to store an object whose size is does not exceed length
// and whose alignment is a divisor of alignment. When using aligned_storage,
// length must be non-zero, and alignment must >= alignment_of<T>::value
// for some type T. We require the alignment value to be a power-of-two.
//
// Example usage:
//     aligned_storage<sizeof(Widget), alignment_of(Widget)>::type widget;
//     Widget* pWidget = new(&widget) Widget;
//
//     aligned_storage<sizeof(Widget), 64>::type widgetAlignedTo64;
//     Widget* pWidget = new(&widgetAlignedTo64) Widget;
//
//     aligned_storage<sizeof(Widget), alignment_of(Widget)>::type widgetArray[37];
//     Widget* pWidgetArray = new(widgetArray) Widget[37];

#if !defined offsetof
#define offsetof(type, field) ((size_t)(&((type *) 0)->field))
#endif

#if COMPILER == GCC || COMPILER == CLANG
// New versions of GCC do not support using 'alignas' with a value greater than 128.
template <size_t N, size_t Align = 8>
struct aligned_storage {
    struct type {
        u8 Data[N];
    } __attribute__((aligned(Align)));
};
#else
template <size_t N, size_t Align = 8>
struct aligned_storage {
    typedef struct {
        alignas(Align) u8 Data[N];
    } type;
};
#endif

template <size_t N, size_t Align = 8>
using aligned_storage_t = typename aligned_storage<N, Align>::type;

// :CopyMemory
extern void (*copy_memory)(void *dest, const void *src, size_t num);

// Safely converts between unrelated types that have a binary equivalency.
// This appoach is required by strictly conforming C++ compilers because
// directly using a C or C++ cast between unrelated types is fraught with
// the possibility of undefined runtime behavior due to type aliasing.
//
// Example usage:
//    float f32 = 1.234f;
//    uint32_t n32 = union_cast<uint32_t>(f32);
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
        retrun destValue;
    }
}

// Maps a sequence of any types to void. This utility struct is used in
// template meta programming to simplify compile time reflection mechanisms
// required by the standard library.
template <typename...>
using void_t = void;

template <typename T>
struct underlying_type {
    using type = __underlying_type(T);
};

template <typename T>
using underlying_type_t = typename underlying_type<T>::type;

// If T is TriviallyCopyable and if any two objects of type T with the same
// value have the same object representation, value is true.
template <typename T>
struct has_unique_object_representations
    : public integral_constant<bool, __has_unique_object_representations(remove_cv_t<remove_all_extents_t<T>>)> {};

template <typename T>
constexpr auto has_unique_object_representations_v = has_unique_object_representations<T>::value;

template <typename T>
struct is_signed_helper : public false_t {};

template <>
struct is_signed_helper<signed char> : public true_t {};
template <>
struct is_signed_helper<signed short> : public true_t {};
template <>
struct is_signed_helper<signed int> : public true_t {};
template <>
struct is_signed_helper<signed long> : public true_t {};
template <>
struct is_signed_helper<signed long long> : public true_t {};
template <>
struct is_signed_helper<float> : public true_t {};
template <>
struct is_signed_helper<double> : public true_t {};
template <>
struct is_signed_helper<long double> : public true_t {};

template <>
struct is_signed_helper<char> : public true_t {};

template <typename T>
struct is_signed : public is_signed_helper<typename remove_cv_t<T>> {};

template <typename T>
constexpr bool is_signed_v = is_signed<T>::value;

// Use this macro to declare your custom type as a signed type
#define DECLARE_SIGNED(T)                                  \
    LSTD_BEGIN_NAMESPACE                                   \
    template <>                                            \
    struct is_signed<T> : public true_t {};                \
    template <>                                            \
    struct is_signed<const T> : public true_t {};          \
    template <>                                            \
    struct is_signed<volatile T> : public true_t {};       \
    template <>                                            \
    struct is_signed<const volatile T> : public true_t {}; \
    LSTD_END_NAMESPACE

template <typename T>
struct is_unsigned_helper : public false_t {};

template <>
struct is_unsigned_helper<unsigned char> : public true_t {};
template <>
struct is_unsigned_helper<unsigned short> : public true_t {};
template <>
struct is_unsigned_helper<unsigned int> : public true_t {};
template <>
struct is_unsigned_helper<unsigned long> : public true_t {};
template <>
struct is_unsigned_helper<unsigned long long> : public true_t {};

#if defined _NATIVE_WCHAR_T_DEFINED
template <>
struct is_unsigned_helper<wchar_t> : public true_t {};
#endif

template <typename T>
struct is_unsigned : public is_unsigned_helper<typename remove_cv_t<T>> {};

template <typename T>
constexpr bool is_unsigned_v = is_unsigned<T>::value;

// Use this macro to declare your custom type as a signed type
#define DECLARE_UNSIGNED(T)                                  \
    LSTD_BEGIN_NAMESPACE                                     \
    template <>                                              \
    struct is_unsigned<T> : public true_t {};                \
    template <>                                              \
    struct is_unsigned<const T> : public true_t {};          \
    template <>                                              \
    struct is_unsigned<volatile T> : public true_t {};       \
    template <>                                              \
    struct is_unsigned<const volatile T> : public true_t {}; \
    LSTD_END_NAMESPACE

template <typename T>
struct alignment_of_value {
    static const size_t value = alignof(T);
};

template <typename T>
struct alignment_of : public integral_constant<size_t, alignment_of_value<T>::value> {};

template <typename T>
constexpr size_t alignment_of_v = alignment_of<T>::value;

template <typename T>
struct is_aligned_value {
    static constexpr auto value = (alignof(T) > 8);
};

template <typename T>
struct is_aligned : public integral_constant<bool, is_aligned_value<T>::value> {};

template <typename T>
constexpr size_t is_aligned_v = is_aligned<T>::value;

template <typename T>
struct rank : public integral_constant<size_t, 0> {};

template <typename T>
struct rank<T[]> : public integral_constant<size_t, rank<T>::value + 1> {};

template <typename T, size_t N>
struct rank<T[N]> : public integral_constant<size_t, rank<T>::value + 1> {};

template <typename T>
constexpr size_t rank_v = rank<T>::value;

template <typename Base, typename Derived>
struct is_base_of : public integral_constant<bool, __is_base_of(Base, Derived) || is_same<Base, Derived>::value> {};

template <typename Base, typename Derived>
constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;

template <typename T>
struct is_lvalue_reference : public false_t {};
template <typename T>
struct is_lvalue_reference<T &> : public true_t {};

template <typename T>
constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

template <typename T>
struct is_rvalue_reference : public false_t {};
template <typename T>
struct is_rvalue_reference<T &&> : public true_t {};

template <typename T>
constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

template <typename>
struct result_of;

template <typename F, typename... ArgTypes>
struct result_of<F(ArgTypes...)> {
    typedef decltype(declval<F>()(declval<ArgTypes>()...)) type;
};

template <typename T>
using result_of_t = typename result_of<T>::type;

//  Determines if the specified type can be tested for equality.
template <typename, typename = void_t<>>
struct has_equality : false_t {};

template <typename T>
struct has_equality<T, void_t<decltype(declval<T>() == declval<T>())>> : true_t {};

template <typename T>
constexpr auto has_equality_v = has_equality<T>::value;

// An integral type representing the number of elements in the Ith dimension of array type T.
//
// For a given array type T[N], extent<T[N]>::value == N.
// For a given multi-dimensional array type T[M][N], extent<T[M][N], 0>::value == N.
// For a given multi-dimensional array type T[M][N], extent<T[M][N], 1>::value == M.
// For a given array type T and a given dimension I where I >= rank<T>::value, extent<T, I>::value == 0.
// For a given array type of unknown extent T[], extent<T[], 0>::value == 0.
// For a given non-array type T and an arbitrary dimension I, extent<T, I>::value == 0.
template <typename T, u32 N>
struct extent_help : public integral_constant<size_t, 0> {};

template <typename T, u32 I>
struct extent_help<T[I], 0> : public integral_constant<size_t, I> {};

template <typename T, u32 N, u32 I>
struct extent_help<T[I], N> : public extent_help<T, N - 1> {};

template <typename T, u32 N>
struct extent_help<T[], N> : public extent_help<T, N - 1> {};

template <typename T, u32 N = 0>
struct extent : public extent_help<T, N> {};

template <typename T, u32 N = 0>
constexpr auto extent_v = extent<T, N>::value;

template <typename T>
struct is_array : public false_t {};

template <typename T>
struct is_array<T[]> : public true_t {};

template <typename T, size_t N>
struct is_array<T[N]> : public true_t {};

template <typename T>
constexpr bool is_array_v = is_array<T>::value;

// Not part of the C++ Standard.
template <typename T>
struct is_array_of_known_bounds : public integral_constant<bool, extent_v<T> != 0> {};

template <typename T>
constexpr bool is_array_of_known_bounds_v = is_array_of_known_bounds<T>::value;

// Not part of the C++ Standard.
template <typename T>
struct is_array_of_unknown_bounds : public integral_constant<bool, is_array_v<T> && (extent_v<T> == 0)> {};

template <typename T>
constexpr bool is_array_of_unknown_bounds_v = is_array_of_unknown_bounds<T>::value;

template <typename T>
struct is_mem_fun_pointer_value : public false_t {};

template <typename R, typename T, typename... Args>
struct is_mem_fun_pointer_value<R (T::*)(Args...)> : public true_t {};
template <typename R, typename T, typename... Args>
struct is_mem_fun_pointer_value<R (T::*)(Args...) const> : public true_t {};
template <typename R, typename T, typename... Args>
struct is_mem_fun_pointer_value<R (T::*)(Args...) volatile> : public true_t {};
template <typename R, typename T, typename... Args>
struct is_mem_fun_pointer_value<R (T::*)(Args...) const volatile> : public true_t {};

template <typename T>
struct is_member_function_pointer : public integral_constant<bool, is_mem_fun_pointer_value<T>::value> {};

template <typename T>
constexpr bool is_member_function_pointer_v = is_member_function_pointer<T>::value;

template <typename T>
struct is_member_pointer : public integral_constant<bool, is_member_function_pointer<T>::value> {};

template <typename T, typename U>
struct is_member_pointer<U T::*> : public true_t {};

template <typename T>
constexpr bool is_member_pointer_v = is_member_pointer<T>::value;

template <typename T>
struct is_member_object_pointer
    : public integral_constant<bool, is_member_pointer_v<T> && !is_member_function_pointer_v<T>> {};
template <typename T>
constexpr bool is_member_object_pointer_v = is_member_object_pointer<T>::value;

// Doesn't include pointers to member types (use the tests above for that)
template <typename T>
struct is_pointer_helper : public false_t {};

template <typename T>
struct is_pointer_helper<T *> : public true_t {};
template <typename T>
struct is_pointer_helper<T *const> : public true_t {};
template <typename T>
struct is_pointer_helper<T *volatile> : public true_t {};
template <typename T>
struct is_pointer_helper<T *const volatile> : public true_t {};

template <typename T>
struct is_pointer_value : public type_and<is_pointer_helper<T>::value, type_not<is_member_pointer_v<T>>::value> {};

template <typename T>
struct is_pointer : public integral_constant<bool, is_pointer_value<T>::value> {};

template <typename T>
constexpr bool is_pointer_v = is_pointer<T>::value;

template <typename From, typename To>
struct is_convertible : public integral_constant<bool, __is_convertible_to(From, To)> {};

template <typename From, typename To>
constexpr bool is_convertible_v = is_convertible<From, To>::value;

template <typename T>
struct is_union : public integral_constant<bool, __is_union(T)> {};
template <typename T>

constexpr bool is_union_v = is_union<T>::value;

// Use this macro to declare your type as an union
#define DECLARE_UNION(T)                         \
    LSTD_BEGIN_NAMESPACE                         \
    template <>                                  \
    struct is_union<T> : public true_t {};       \
    template <>                                  \
    struct is_union<const T> : public true_t {}; \
    LSTD_END_NAMESPACE

template <typename T>
struct is_class : public integral_constant<bool, __is_class(T)> {};

template <typename T>
constexpr bool is_class_v = is_class<T>::value;

template <typename T>
struct is_enum : public integral_constant<bool, __is_enum(T)> {};
template <typename T>
constexpr bool is_enum_v = is_enum<T>::value;

// Use this macro to declare your type as an enum
#define DECLARE_ENUM(T)                         \
    LSTD_BEGIN_NAMESPACE                        \
    template <>                                 \
    struct is_enum<T> : public true_t {};       \
    template <>                                 \
    struct is_enum<const T> : public true_t {}; \
    LSTD_END_NAMESPACE

template <typename T>
struct is_object : public integral_constant<bool, !is_reference_v<T> && !is_void_v<T> && !is_function_v<T>> {};

template <typename T>
constexpr bool is_object_v = is_object<T>::value;

template <typename T>
struct is_scalar : public integral_constant<bool, is_arithmetic_v<T> || is_enum_v<T> || is_pointer_v<T> ||
                                                      is_member_pointer_v<T> || is_null_pointer_v<T>> {};

template <typename T>
struct is_scalar<T *> : public true_t {};
template <typename T>
struct is_scalar<T *const> : public true_t {};
template <typename T>
struct is_scalar<T *volatile> : public true_t {};
template <typename T>
struct is_scalar<T *const volatile> : public true_t {};

template <typename T>
constexpr bool is_scalar_v = is_scalar<T>::value;

template <typename T>
struct is_compound : public integral_constant<bool, !is_fundamental<T>::value> {};

template <typename T>
constexpr bool is_compound_v = is_compound<T>::value;

// Converts the type T to its decayed equivalent. That means doing
// lvalue to rvalue, array to pointer, function to pointer conversions,
// and removal of const and volatile.
// This is the type conversion silently applied by the compiler to
// all function arguments when passed by value.
template <typename T>
struct decay {
    using U = typename remove_reference_t<T>;

    using type = typename type_select_t<
        is_array_v<U>, typename remove_extent_t<U> *,
        typename type_select_t<is_function_v<U>, typename add_pointer_t<U>, typename remove_cv_t<U>>>;
};

template <typename T>
using decay_t = typename decay<T>::type;

// Determines the common type among all types T..., that is the type all T...
// can be implicitly converted to.
//
// It is intended that this be specialized by the user for cases where it
// is useful to do so. Example specialization:
//     template <typename Class1, typename Class2>
//     struct common_type<MyClass1, MyClass2>{ typedef MyBaseClassB type; };
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

// An aggregate is one of the following types:
// * array type
// * struct or union, that has
//     * no private or protected non-static data members
//     * no user-provided constructors (explicitly defaulted or deleted constructors are allowed)
//     * no user-provided, inherited, or explicit constructors
//         * (explicitly defaulted or deleted constructors are allowed)
//     * no virtual, private, or protected (since C++17) base classes
//     * no virtual member functions
//     * no default member initializers
template <typename T>
struct is_aggregate : public integral_constant<bool, __is_aggregate(T)> {};

template <typename T>
constexpr bool is_aggregate_v = is_aggregate<T>::value;

template <typename T>
struct is_empty : public integral_constant<bool, __is_empty(T)> {};

template <typename T>
constexpr bool is_empty_v = is_empty<T>::value;

#if COMPILER == MSVC
#pragma warning(push)
#pragma warning(disable : 4647)

template <typename T>
struct is_pod
    : public integral_constant<bool, (__has_trivial_constructor(T) && __is_pod(T)) || is_void_v<T> || is_scalar_v<T>> {
};
#pragma warning(pop)
#else
template <typename T>
struct is_pod : public integral_constant<bool, __is_pod(T) || is_void_v<T> || is_scalar_v<T>> {};
#endif

template <typename T, size_t N>
struct is_pod<T[N]> : public is_pod<T> {};

template <typename T>
constexpr bool is_pod_v = is_pod<T>::value;

// Use this macro to declare your type as POD or not
#define DECLARE_IS_POD(T, isPod)                                                \
    LSTD_BEGIN_NAMESPACE                                                        \
    template <>                                                                 \
    struct is_pod<T> : public integral_constant<bool, isPod> {};                \
    template <>                                                                 \
    struct is_pod<const T> : public integral_constant<bool, isPod> {};          \
    template <>                                                                 \
    struct is_pod<const volatile T> : public integral_constant<bool, isPod> {}; \
    LSTD_END_NAMESPACE

template <typename T>
struct is_standard_layout : public integral_constant<bool, __is_standard_layout(T) || is_void_v<T> || is_scalar_v<T>> {
};

template <typename T>
constexpr bool is_standard_layout_v = is_standard_layout<T>::value;

// A constructor is trivial if
//    - it is implicitly defined by the compiler, and
//    - T has no virtual base classes, and
//    - for every direct base class of T, has_trivial_constructor<B>::value == true,
//      where B is the type of the base class, and
//    - for every nonstatic data member of T that has class type or array
//      of class type, has_trivial_constructor<M>::value == true,
//      where M is the type of the data member
template <typename T>
struct has_trivial_constructor : public integral_constant<bool, (__has_trivial_constructor(T) || is_pod_v<T>)> {};

template <typename T>
constexpr bool has_trivial_constructor_v = has_trivial_constructor<T>::value;

// A copy constructor for class X is trivial if it is implicitly
// declared and if all the following are true:
//    - Class X has no virtual functions (10.3) and no virtual base classes (10.1).
//    - Each direct base class of X has a trivial copy constructor.
//    - For all the nonstatic data members of X that are of class type
//      (or array thereof), each such class type has a trivial copy constructor;
//      otherwise the copy constructor is nontrivial.

#if COMPILER == MSVC
template <typename T>
struct has_trivial_copy : public integral_constant<bool, (__has_trivial_copy(T) || is_pod_v<T>) &&!is_volatile_v<T>> {};
#elif COMPILER == GCC || COMPILER == CLANG
template <typename T>
struct has_trivial_copy : public integral_constant<bool, (__has_trivial_copy(T) ||
                                                          is_pod_v<T>) &&(!is_volatile_v<T> && !is_reference_v<T>)> {};
#endif

template <typename T>
constexpr bool has_trivial_copy_v = has_trivial_copy<T>::value;

template <typename T>
struct has_trivial_assign
    : public integral_constant<bool, (__has_trivial_assign(T) || is_pod_v<T>) &&!is_const_v<T> && !is_volatile_v<T>> {};

template <typename T>
constexpr bool has_trivial_assign_v = has_trivial_assign<T>::value;

template <typename T>
struct has_trivial_destructor : public integral_constant<bool, __has_trivial_destructor(T) || is_pod_v<T>> {};

template <typename T>
constexpr bool has_trivial_destructor_v = has_trivial_destructor<T>::value;

// section 2.9,p10.
// A type is a literal type if it is:
//     - a scalar type; or
//     - a reference type referring to a literal type; or
//     - an array of literal type; or
//     - a class type (Clause 9) that has all of the following properties:
//         - it has a trivial destructor,
//         - every constructor call and full-expression in the brace-or-equal-initializer s for non-static data members
//         (if any) is a constant expression (5.19),
//         - it is an aggregate type (8.5.1) or has at least one constexpr constructor or constructor template that is
//         not a copy or move constructor, and
//         - all of its non-static data members and base classes are of literal types.

#if COMPILER == CLANG
template <typename T>
struct is_literal_type : public integral_constant<bool, __is_literal(T)> {};
#else
template <typename T>
struct is_literal_type : public integral_constant<bool, __is_literal_type(T)> {};
#endif

template <typename T>
constexpr bool is_literal_type_v = is_literal_type<T>::value;

template <typename T>
struct is_trivially_copyable {
    static const bool value = __is_trivially_copyable(T);
};

#define EASTL_DECLARE_IS_TRIVIALLY_COPYABLE(T, isTriviallyCopyable)                                          \
    LSTD_BEGIN_NAMESPACE                                                                                     \
    template <>                                                                                              \
    struct is_trivially_copyable<T> : public integral_constant<bool, isTriviallyCopyable> {};                \
    template <>                                                                                              \
    struct is_trivially_copyable<const T> : public integral_constant<bool, isTriviallyCopyable> {};          \
    template <>                                                                                              \
    struct is_trivially_copyable<volatile T> : public integral_constant<bool, isTriviallyCopyable> {};       \
    template <>                                                                                              \
    struct is_trivially_copyable<const volatile T> : public integral_constant<bool, isTriviallyCopyable> {}; \
    LSTD_END_NAMESPACE

template <typename T>
constexpr bool is_trivially_copyable_v = is_trivially_copyable<T>::value;

#if !COMPILER == MSVC || COMPILER == CLANG
template <typename T, typename... Args>
struct is_constructible : public bool_constant<__is_constructible(T, Args...)> {};
#else
template <typename T>
inline typename remove_reference<T>::type &&move_internal(T &&x) {
    return ((typename remove_reference<T>::type &&) x);
}

template <typename T, typename... Args>
typename first_type_select<true_t, decltype(move_internal(T(declval<Args>()...)))>::type is(T &&, Args &&...);

template <typename T>
struct can_construct_scalar_helper {
    static true_t can(T);
    static false_t can(...);
};

template <typename... Args>
false_t is(argument_sink, Args &&...);

// Except for scalars and references (handled below), check for constructibility via decltype.
template <bool, typename T, typename... Args>
struct is_constructible_helper_2 : public type_identity<decltype(is(declval<T>(), declval<Args>()...))>::type {};

template <typename T>
struct is_constructible_helper_2<true, T> : public is_scalar<T> {};

template <typename T, typename Arg0>  // We handle the case of multiple arguments below (by disallowing them).
struct is_constructible_helper_2<true, T, Arg0>
    : public type_identity<decltype(can_construct_scalar_helper<T>::can(declval<Arg0>()))>::type {};

// Scalars and references can be constructed only with 0 or 1 argument. e.g the following is an invalid expression:
// int(17, 23)
template <typename T, typename Arg0, typename... Args>
struct is_constructible_helper_2<true, T, Arg0, Args...> : public false_t {};

template <bool, typename T, typename... Args>
struct is_constructible_helper_1 : public is_constructible_helper_2<is_scalar_v<T> || is_reference_v<T>, T, Args...> {};

// Unilaterally dismiss void, abstract, unknown bound arrays, and function types as not constructible.
template <typename T, typename... Args>
struct is_constructible_helper_1<true, T, Args...> : public false_t {};

// is_constructible
template <typename T, typename... Args>
struct is_constructible
    : public is_constructible_helper_1<(is_array_of_unknown_bounds_v<T> ||
                                        is_function_v<typename remove_all_extents_t<T>> || has_void_arg_v<T, Args...>),
                                       T, Args...> {};

// Array types are constructible if constructed with no arguments and if their element type is default-constructible
template <typename Array, size_t N>
struct is_constructible_helper_2<false, Array[N]> : public is_constructible<typename remove_all_extents<Array>::type> {
};

// Arrays with arguments are not constructible. e.g. the following is an invalid expression: int[3](37, 34, 12)
template <typename Array, size_t N, typename... Args>
struct is_constructible_helper_2<false, Array[N], Args...> : public false_t {};
#endif

template <typename T, typename... Args>
constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

template <typename T, typename... Args>
struct is_trivially_constructible
    : public integral_constant<bool, is_constructible_v<T, Args...> &&__is_trivially_constructible(T, Args...)> {};

template <typename T, typename... Args>
constexpr bool is_trivially_constructible_v = is_trivially_constructible<T, Args...>::value;

template <typename T>
struct is_trivial : public integral_constant<bool, is_trivially_copyable_v<T> && is_trivially_constructible_v<T>> {};

template <typename T>
constexpr bool is_trivial_v = is_trivial<T>::value;

template <typename T>
struct is_copy_constructible : public is_constructible<T, typename add_lvalue_reference_t<typename add_const_t<T>>> {};

template <typename T>
constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;

template <typename T>
struct is_trivially_copy_constructible
    : public is_trivially_constructible<T, typename add_lvalue_reference_t<typename add_const_t<T>>> {};

template <typename T>
constexpr bool is_trivially_copy_constructible_v = is_trivially_copy_constructible<T>::value;

template <typename T>
struct is_move_constructible : public is_constructible<T, typename add_rvalue_reference_t<T>> {};

template <typename T>
constexpr bool is_move_constructible_v = is_move_constructible<T>::value;

template <typename T>
struct is_trivially_move_constructible : public is_trivially_constructible<T, typename add_rvalue_reference_t<T>> {};

template <typename T>
constexpr bool is_trivially_move_constructible_v = is_trivially_move_constructible<T>::value;

// The expression declval<T>() = declval<U>() is well-formed when treated as an unevaluated operand.
// Access checking is performed as if in a context unrelated to T and U. Only the validity of
// the immediate context of the assignment expression is considered. The compilation of the expression
// can result in side effects such as the instantiation of class template specializations and function
// template specializations, the generation of implicitly-defined functions, and so on. Such side
// effects are not in the "immediate context" and can result in the program being ill-formed.
//
// Note:
// This type trait has a misleading and counter-intuitive name. It does not indicate whether an instance
// of U can be assigned to an instance of T (e.g. t = u). Instead it indicates whether the assignment can be
// done after adding rvalue references to both, as in add_rvalue_reference<T>::type = add_rvalue_reference<U>::type.
// A counterintuitive result of this is that is_assignable<int, int>::value == false. The is_copy_assignable
// trait indicates if a type can be assigned to its own type, though there isn't a standard C++ way to tell
// if an arbitrary type is assignable to another type.
// http://stackoverflow.com/questions/19920213/why-is-stdis-assignable-counter-intuitive
//
// Note:
// A true is_assignable value doesn't guarantee that the expression is compile-able, the compiler checks
// only that the assignment matches before compilation. In particular, if you have templated operator=
// for a class, the compiler will always say is_assignable is true, regardless of what's being tested
// on the right hand side of the expression. It may actually turn out during compilation that the
// templated operator= fails to compile because in practice it doesn't accept every possible type for
// the right hand side of the expression.
//
// Expected results:
//     is_assignable<void, void>::value             == false
//     is_assignable<int&, int>::value              == true
//     is_assignable<int, int>::value               == false
//     is_assignable<int, int&>::value              == false
//     is_assignable<bool, bool>::value             == false
//     is_assignable<int, float>::value             == false
//     is_assignable<int[], int[]>::value           == false
//     is_assignable<char*, int*>::value            == false
//     is_assignable<char*, const char*>::value     == false
//     is_assignable<const char*, char*>::value     == false
//     is_assignable<PodA, PodB*>::value            == false
//     is_assignable<Assignable, Assignable>::value == true
//     is_assignable<Assignable, Unrelated>::value  == false
template <typename T, typename U>
struct is_assignable_helper {
    template <typename, typename>
    static no_t is(...);

    template <typename T1, typename U1>
    static decltype(declval<T1>() = declval<U1>(), yes_t()) is(int);

    static constexpr auto value = (sizeof(is<T, U>(0)) == sizeof(yes_t));
};

template <typename T, typename U>
struct is_assignable : public integral_constant<bool, is_assignable_helper<T, U>::value> {};

template <typename T, typename U>
constexpr bool is_assignable_v = is_assignable<T, U>::value;

template <typename T, typename U>
struct is_lvalue_assignable : public is_assignable<typename add_lvalue_reference_t<T>,
                                                   typename add_lvalue_reference_t<typename add_const_t<U>>> {};

template <typename T, typename U>
constexpr bool is_lvalue_assignable_v = is_lvalue_assignable<T, U>::value;

#if COMPILER == CLANG
template <typename T, typename U>
struct is_trivially_assignable : integral_constant<bool, __is_trivially_assignable(T, U)> {};
#elif COMPILER == MSVC
template <bool A, typename T, typename U>
struct is_trivially_assignable_helper;

template <typename T, typename U>
struct is_trivially_assignable_helper<true, T, U> : integral_constant<bool, __is_trivially_assignable(T, U)> {};

template <typename T, typename U>
struct is_trivially_assignable_helper<false, T, U> : false_t {};

template <typename T, typename U>
struct is_trivially_assignable
    : integral_constant<bool, is_trivially_assignable_helper<is_assignable_v<T, U>, T, U>::value> {};

#else
template <typename T, typename U>
struct is_trivially_assignable
    : integral_constant<bool, is_assignable_v<T, U> && (is_pod_v<typename remove_reference_t<T>> ||
                                                        __has_trivial_assign(typename remove_reference_t<T>))> {};

#endif

template <typename T, typename U>
constexpr bool is_trivially_assignable_v = is_trivially_assignable<T, U>::value;

template <typename T>
struct is_copy_assignable : public is_assignable<typename add_lvalue_reference_t<T>,
                                                 typename add_lvalue_reference_t<typename add_const_t<T>>> {};

template <typename T>
constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;

template <typename T>
struct is_trivially_copy_assignable
    : public is_trivially_assignable<typename add_lvalue_reference_t<T>,
                                     typename add_lvalue_reference_t<typename add_const_t<T>>> {};

template <typename T>
constexpr bool is_trivially_copy_assignable_v = is_trivially_copy_assignable<T>::value;

template <typename T>
struct is_move_assignable
    : public is_assignable<typename add_lvalue_reference_t<T>, typename add_rvalue_reference_t<T>> {};
template <class T>
constexpr bool is_move_assignable_v = is_move_assignable<T>::value;

template <typename T>
struct is_trivially_move_assignable
    : public is_trivially_assignable<typename add_lvalue_reference_t<T>, typename add_rvalue_reference_t<T>> {};

template <typename T>
constexpr bool is_trivially_move_assignable_v = is_trivially_move_assignable<T>::value;

template <typename U>
struct destructible_test_helper {
    U u;
};

template <typename>
false_t destructible_test_function(...);

template <typename T, typename U = decltype(declval<destructible_test_helper<T>>().~destructible_test_helper<T>())>
true_t destructible_test_function(int);

template <typename T, bool = is_array_of_unknown_bounds_v<T> || is_void_v<T> || is_function_v<T>>
struct is_destructible_helper : public type_identity_t<decltype(destructible_test_function<T>(0))> {
};  // Need to wrap decltype with identity because some compilers otherwise don't like the bare decltype usage.

template <typename T>
struct is_destructible_helper<T, true> : public false_t {};

template <typename T>
struct is_destructible : public is_destructible_helper<T> {};

template <typename T>
constexpr bool is_destructible_v = is_destructible<T>::value;

template <typename T>
struct is_trivially_destructible_helper
    : public integral_constant<bool, (is_pod_v<T> || is_scalar_v<T> || is_reference_v<T>) &&!is_void_v<T>> {};

template <typename T>
struct is_trivially_destructible : public is_trivially_destructible_helper<typename remove_all_extents_t<T>> {};

template <typename T>
constexpr bool is_trivially_destructible_v = is_trivially_destructible<T>::value;

LSTD_END_NAMESPACE
