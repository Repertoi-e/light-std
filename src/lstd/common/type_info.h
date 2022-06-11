#pragma once

#include "cpp/space_ship.h"
#include "ieee.h"

//
// This file defines the following types:
// - integral_constant (a struct with a type and compile-time value, usually in the context of integrals)
// - true_t, false_t (integral_constant<bool, true/false>, this is == to std::true_type, std::false_type)
// - unused (a dummy type used in templates because C++ is dumb)
//
// - select (select from two types based on a condition, this is == to std::conditional)
//
// Concepts:
// - is_same (checks if two types are the same)
// - is_same_template (checks if two types are the same, regardless of their template parameters - doesn't work if you mix types and typenames...)
// - is_const, is_pointer, is_member_pointer
// - is_void, is_null (decltype(null))
// - is_integral, is_signed_integral, is_unsigned_integral, 
//   is_floating_point, is_arithmetic, is_enum, is_scalar
// - is_constructible, is_convertible
//
// Info about arrays:
// - extent_v (returns the size of the nth extent of an array, 
//             e.g. extent_v<s32[2][4][9], 1> -> 4)
// - is_array
// - is_array_of_known_bounds (not part of the C++ std)
//
// Transformations:
// - remove_const, remove_volatile, remove_cv, remove_reference, remove_cvref
// - add_rvalue_reference, declval
// - underlying_type (returns the underlying integral type on the enum)
//
// - remove_pointer/add_pointer
// - remove_extent (for arrays, e.g. s32[][] -> s32[])
//
// - decay (applies lvalue-to-rvalue, array-to-pointer, function-to-pointer 
//          implicit conversions to the type T, removes cv-qualifiers)
// - common_type (determines the common type among all types T..., that is 
//                the type all T... can be implicitly converted to, can be 
//                explictly specialized by the user)
//
// - bit_cast
//

LSTD_BEGIN_NAMESPACE

namespace types {

//
// Safely converts between unrelated types that have a binary equivalency.
// This approach is required by strictly conforming C++ compilers because
// directly using a C or C++ cast between unrelated types is fraught with
// the possibility of undefined runtime behavior due to type aliasing.
//
// Example usage:
//    f32 f = 1.234f;
//    u32 br = bit_cast<u32>(f);
//
template <typename DestType, typename SourceType>
constexpr DestType bit_cast(SourceType const & sourceValue) {
	static_assert(sizeof(DestType) == sizeof(SourceType));
    return __builtin_bit_cast(DestType, sourceValue);
}

// This is essentially a utility base struct for defining properties as both struct constants and as types.
template <typename T, T Value>
struct integral_constant {
    static constexpr T value = Value;

    using value_t = T;
    using type    = integral_constant<T, Value>;

    constexpr operator value_t() const { return value; }
    constexpr value_t operator()() const { return value; }
};

using true_t  = integral_constant<bool, true>;   // == to std::true_type
using false_t = integral_constant<bool, false>;  // == to std::false_type

// Used to denote a special template argument that means it's an unused argument
struct unused {
};

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

//
// Checks if two types are the same
//
template <typename T, typename U>
struct same_helper : false_t {
};

template <typename T>
struct same_helper<T, T> : true_t {
};

template <typename T, typename U>
concept is_same = same_helper<T, U>::value;

template <typename T, typename... Types>
concept is_same_to_one_of = (is_same<T, Types> || ...);

//
// Checks if two types are the same, regardless of their template parameters.
// Doesn't work if you mix types and typenames...
// but we provide a specialization for stuff with the signature of a stack_array - typename and s64
// which is one of the most common cases.
//
template <typename, typename>
constexpr bool is_same_template_helper = false;

template <template <typename...> typename T, typename... A, typename... B>
constexpr bool is_same_template_helper<T<A...>, T<B...>> = true;

template <typename T, typename U>
concept is_same_template = is_same_template_helper<T, U>;

// Are the decayed versions of "T" and "U" the same basic type?
// Gets around the fact that is_same will treat, say "bool" and "bool&" as different types.
template <typename T, typename U>
concept is_same_decayed = same_helper<decay_t<T>, decay_t<U>>::value;

// Checks if T has const-qualification
template <typename T>
struct is_const_helper_1 : false_t {
};

template <typename T>
struct is_const_helper_1<volatile T *> : true_t {
};

template <typename T>
struct is_const_helper_1<const volatile T *> : true_t {
};

template <typename T>
struct is_const_helper_2 : is_const_helper_1<T *> {
};

template <typename T>
struct is_const_helper_2<T &> : false_t {
};  // Note here that T is const, not the reference to T. So is_const is false.

template <typename T>
concept is_const = is_const_helper_2<T>::value;

template <typename T>
struct is_void_helper : false_t {
};

template <>
struct is_void_helper<void> : true_t {
};

template <typename T>
concept is_void = is_void_helper<remove_cv_t<T>>::value;

template <typename T>
concept is_null = is_same < remove_cv_t<T>,
decltype(null) > ;

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

template <typename T, s64 N>
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

template <typename T, s64 N>
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

template <typename T, typename U>
concept is_same_template_decayed = is_same_template<remove_cv_t<T>, remove_cv_t<U>>;

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
// remove_cv<s32& volatile>::type is equivalent to s32.
template <typename T>
struct remove_cvref {
    using type = remove_volatile_t<remove_const_t<remove_reference_t<T>>>;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

//
// Concept satisfied if T is one of the following types: bool, char, wchar_t, s16, s32, s64 (and unsigned variants)
//
template <typename T>
struct is_integral_helper : false_t {
};

template <>
struct is_integral_helper<unsigned char> : true_t {
};

template <>
struct is_integral_helper<unsigned short> : true_t {
};

template <>
struct is_integral_helper<unsigned int> : true_t {
};

template <>
struct is_integral_helper<unsigned long> : true_t {
};

template <>
struct is_integral_helper<unsigned long long> : true_t {
};

template <>
struct is_integral_helper<signed char> : true_t {
};

template <>
struct is_integral_helper<signed short> : true_t {
};

template <>
struct is_integral_helper<signed int> : true_t {
};

template <>
struct is_integral_helper<signed long> : true_t {
};

template <>
struct is_integral_helper<signed long long> : true_t {
};

template <>
struct is_integral_helper<bool> : true_t {
};

template <>
struct is_integral_helper<char> : true_t {
};

template <>
struct is_integral_helper<char8_t> : true_t {
};

template <>
struct is_integral_helper<char16_t> : true_t {
};

template <>
struct is_integral_helper<char32_t> : true_t {
};

#if defined _NATIVE_WCHAR_T_DEFINED
template <>
struct is_integral_helper<wchar_t> : true_t {
};
#endif

template <typename T>
concept is_integral = is_integral_helper<remove_cv_t<T>>::value;

template <typename T>
concept is_signed_integral = is_integral<T> && T(-1)
< T(0);

template <typename T>
concept is_unsigned_integral = is_integral<T> && !is_signed_integral<T>;

//
// Concept satisfied if T is float, double
//
template <typename T>
struct is_floating_point_helper : false_t {
};

template <>
struct is_floating_point_helper<float> : true_t {
};

template <>
struct is_floating_point_helper<double> : true_t {
};

// Use this macro to declare your custom type as a floating point
#define DECLARE_FLOATING_POINT(T)                   \
    LSTD_BEGIN_NAMESPACE                            \
    template <>                                     \
    struct is_floating_point_helper<T> : true_t {}; \
    LSTD_END_NAMESPACE

template <typename T>
concept is_floating_point = is_floating_point_helper<remove_cv_t<T>>::value;

//
// An arithmetic type is an integral type or a floating point type
//
template <typename T>
concept is_arithmetic = is_integral<T> || is_floating_point<T>;

template <typename T>
concept is_enum = __is_enum(remove_cv_t<T>);

// Checks if T is a function type (doesn't include member functions)
template <typename>
struct is_function_helper : false_t {
};

template <typename R, typename... Args>
struct is_function_helper<R(Args...)> : true_t {
};

template <typename R, typename... Args>
struct is_function_helper<R(Args..., ...)> : true_t {
};

template <typename T>
concept is_function = is_function_helper<T>::value;

// Tests whether T is a pointer to an object, to a function, but not to member objects/functions (use the tests above for that)
//
template <typename T>
struct is_pointer_helper : false_t {
};

template <typename T>
struct is_pointer_helper<T *> : true_t {
};

template <typename T>
concept is_pointer = is_pointer_helper<remove_cv_t<T>>::value;

//
// True if T is a pointer to a member object OR a member function
//
template <typename T>
struct is_member_pointer_helper : false_t {
};

template <typename T, typename U>
struct is_member_pointer_helper<T U::*> : true_t {
};

template <typename T>
concept is_member_pointer = is_member_pointer_helper<remove_cv_t<T>>::value;

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
struct underlying_type_helper {
    using type = __underlying_type(T);
};

template <typename T>
using underlying_type_t = typename underlying_type_helper<T>::type;

//
// Stuff to do with arrays:
//

// An integral type representing the number of elements in the Ith dimension of array type T.
//
// For a given array type T[N], extent<T[N]>::value == N.
// For a given multi-dimensional array type T[M][N], extent<T[M][N], 0>::value == N.
// For a given multi-dimensional array type T[M][N], extent<T[M][N], 1>::value == M.
// For a given array type T and a given dimension I where I >= rank<T>::value, extent<T, I>::value == 0.
// For a given array type of unknown extent T[], extent<T[], 0>::value == 0.
// For a given non-array type T and an arbitrary dimension I, extent<T, I>::value == 0.
template <typename T, s64 N>
struct extent_helper : integral_constant<s64, 0> {
};

template <typename T, s64 I>
struct extent_helper<T[I], 0> : integral_constant<s64, I> {
};

template <typename T, s64 N, s64 I>
struct extent_helper<T[I], N> : extent_helper<T, N - 1> {
};

template <typename T, s64 N>
struct extent_helper<T[], N> : extent_helper<T, N - 1> {
};

template <typename T, s64 N = 0>
constexpr s64 extent_v = extent_helper<T, N>::value;

template <typename T>
struct is_array_helper : false_t {
};

template <typename T>
struct is_array_helper<T[]> : true_t {
};

template <typename T, int N>
struct is_array_helper<T[N]> : true_t {
};

template <typename T>
concept is_array = is_array_helper<T>::value;

// Not part of the C++ Standard.
template <typename T>
struct is_array_of_known_bounds : integral_constant<bool, extent_v<T> != 0> {
};

template <typename T>
constexpr bool is_array_of_known_bounds_v = is_array_of_known_bounds<T>::value;

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

// Rules (8.3.2 p6):
//      void + &&  -> void
//      T    + &&  -> T&&
//      T&   + &&  -> T&
//      T&&  + &&  -> T&&
template <typename T>
struct add_rvalue_reference {
	using type = T&&;
};

template <typename T>
struct add_rvalue_reference<T&> {
	using type = T&;
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

// Converts the type T to its decayed equivalent. That means doing
// lvalue to rvalue, array to pointer, function to pointer conversions,
// and removal of const and volatile.
// This is the type conversion silently applied by the compiler to
// all function arguments when passed by value.
template <typename T>
struct decay {
    using U = remove_reference_t<T>;

    using type = select_t<is_array<U>,
                          remove_extent_t<U> *,
                          select_t<is_function<U>,
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

LSTD_END_NAMESPACE

// Use this macro to declare your custom type as an integral
#define DECLARE_INTEGRAL(T)                   \
    LSTD_BEGIN_NAMESPACE                      \
    namespace types {                         \
    template <>                               \
    struct is_integral_helper<T> : true_t {}; \
    }                                         \
    LSTD_END_NAMESPACE
