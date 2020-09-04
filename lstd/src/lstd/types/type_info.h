#pragma once

#include "../internal/namespace.h"
#include "../platform.h"

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

// Used internally to denote a special template argument that means it's an unused argument.
struct unused {};

// Used as a type which constructs from anything
struct argument_sink {
    template <typename... Args>
    argument_sink(Args &&...) {}
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
//
//

// The purpose of this is typically to deal with non-deduced template contexts
template <typename T>
struct type_identity { using type = T; };

template <typename T>
using type_identity_t = typename type_identity<T>::type;

//
// Checks if two types are the same
//
template <typename T, typename U>
struct same_helper : public false_t {};

template <typename T>
struct same_helper<T, T> : public true_t {};

template <typename T, typename U>
concept is_same = same_helper<T, U>::value;

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

// Checks if T is a function type (doesn't include member functions)
template <typename>
struct is_function_helper : public false_t {};

template <typename R, typename... Args>
struct is_function_helper<R(Args...)> : public true_t {};

template <typename R, typename... Args>
struct is_function_helper<R(Args..., ...)> : public true_t {};

template <typename T>
concept is_function = is_function_helper<T>::value;

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

template <typename T>
struct is_void_helper : public false_t {};

template <>
struct is_void_helper<void> : public true_t {};

template <typename T>
concept is_void = is_void_helper<remove_cv_t<T>>::value;

template <typename T>
concept is_null = is_same<remove_cv_t<T>, decltype(null)>;

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
// Concept satisfied if T is float, double, long double
//
template <typename T>
struct is_floating_point_helper : public false_t {};

template <>
struct is_floating_point_helper<float> : public true_t {};
template <>
struct is_floating_point_helper<double> : public true_t {};
template <>
struct is_floating_point_helper<long double> : public true_t {};

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
concept object = !is_reference_v<T> && !is_void_v<T> && !is_function_v<T>;

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

/*
template <typename T>
struct alignment_of_value {
    static const int value = alignof(T);
};

template <typename T>
struct alignment_of : public integral_constant<int, alignment_of_value<T>::value> {};

template <typename T>
constexpr int alignment_of_v = alignment_of<T>::value;

template <typename T>
struct is_aligned_value {
    static constexpr auto value = (alignof(T) > 8);
};

template <typename T>
struct is_aligned : public integral_constant<bool, is_aligned_value<T>::value> {};

template <typename T>
constexpr int is_aligned_v = is_aligned<T>::value;



template <typename>
struct result_of;

template <typename, typename...>
struct invoke_result;

template <typename F, typename... ArgTypes>
struct result_of<F(ArgTypes...)> {
    using type = decltype(declval<F>()(declval<ArgTypes>()...));
};

template <typename F, typename... ArgTypes>
struct invoke_result<F(ArgTypes...)> {
    using type = decltype(declval<F>()(declval<ArgTypes>()...));
};

template <typename T>
using result_of_t = typename result_of<T>::type;

template <typename T, typename... ArgTypes>
using invoke_result_t = typename invoke_result<T, ArgTypes...>::type;
*/

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

//

template <typename From, typename To>
struct is_convertible : public integral_constant<bool, __is_convertible_to(From, To)> {};

template <typename From, typename To>
constexpr bool is_convertible_v = is_convertible<From, To>::value;

//

template <typename T, typename... Args>
struct is_constructible : public integral_constant<bool, __is_constructible(T, Args...)> {};

template <typename T, typename... Args>
constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

/*
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
    : public integral_constant<bool, (__has_trivial_constructor(T) && __is_pod(T)) || is_void_v<T> || is_scalar<T>> {
};
#pragma warning(pop)
#else
template <typename T>
struct is_pod : public integral_constant<bool, __is_pod(T) || is_void_v<T> || is_scalar<T>> {};
#endif

template <typename T, int N>
struct is_pod<T[N]> : public is_pod<T> {};

template <typename T>
struct is_pod<const T> : is_pod<T> {};

template <typename T>
struct is_pod<const volatile T> : is_pod<T> {};

template <typename T>
constexpr bool is_pod_v = is_pod<T>::value;

// Use this macro to declare your type as POD or not
#define DECLARE_IS_POD(T, isPod)                                 \
    LSTD_BEGIN_NAMESPACE                                         \
    template <>                                                  \
    struct is_pod<T> : public integral_constant<bool, isPod> {}; \
    LSTD_END_NAMESPACE

template <typename T>
struct is_standard_layout : public integral_constant<bool, __is_standard_layout(T) || is_void_v<T> || is_scalar<T>> {
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
*/
/*
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
struct is_copy_constructible : public is_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {};

template <typename T>
constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;

template <typename T>
struct is_trivially_copy_constructible : public is_trivially_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {
};

template <typename T>
constexpr bool is_trivially_copy_constructible_v = is_trivially_copy_constructible<T>::value;

template <typename T>
struct is_move_constructible : public is_constructible<T, add_rvalue_reference_t<T>> {};

template <typename T>
constexpr bool is_move_constructible_v = is_move_constructible<T>::value;

template <typename T>
struct is_trivially_move_constructible : public is_trivially_constructible<T, add_rvalue_reference_t<T>> {};

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
struct is_lvalue_assignable : public is_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<add_const_t<U>>> {
};

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
struct is_copy_assignable : public is_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<add_const_t<T>>> {};

template <typename T>
constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;

template <typename T>
struct is_trivially_copy_assignable
    : public is_trivially_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<add_const_t<T>>> {};

template <typename T>
constexpr bool is_trivially_copy_assignable_v = is_trivially_copy_assignable<T>::value;

template <typename T>
struct is_move_assignable : public is_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {};
template <typename T>
constexpr bool is_move_assignable_v = is_move_assignable<T>::value;

template <typename T>
struct is_trivially_move_assignable
    : public is_trivially_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {};

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
    : public integral_constant<bool, (is_pod_v<T> || is_scalar<T> || is_reference_v<T>) &&!is_void_v<T>> {};

template <typename T>
struct is_trivially_destructible : public is_trivially_destructible_helper<remove_all_extents_t<T>> {};

template <typename T>
constexpr bool is_trivially_destructible_v = is_trivially_destructible<T>::value;
*/
}  // namespace types

LSTD_END_NAMESPACE
