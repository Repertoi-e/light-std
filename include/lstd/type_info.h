#pragma once

#include "common.h"

LSTD_BEGIN_NAMESPACE

/**
 * @brief A utility base struct for defining properties as both
 * structconstants and as types.
 *
 * @tparam T The type of the integral constant.
 * @tparam Value The value of the integral constant.
 */
template <typename T, T Value>
struct integral_constant {
  static const T value = Value;

  using value_t = T;
  using type = integral_constant<T, Value>;

  operator value_t() const { return value; }
  value_t operator()() const { return value; }
};

/// @brief A type alias equivalent to integral_constant<bool, true>
using true_t = integral_constant<bool, true>;
/// @brief A type alias equivalent to integral_constant<bool, false>
using false_t = integral_constant<bool, false>;

namespace internal {
template <typename T>
struct is_const_helper_1 : false_t {};
template <typename T>
struct is_const_helper_1<volatile T *> : true_t {};
template <typename T>
struct is_const_helper_1<const volatile T *> : true_t {};
template <typename T>
struct is_const_helper_2 : is_const_helper_1<T *> {};
template <typename T>
struct is_const_helper_2<T &> : false_t {
};  // Note here that Tis const, not the reference to T. So is_const is false.

template <typename, typename>
bool is_same_template_helper = false;
template <template <typename...> typename T, typename... A, typename... B>
bool is_same_template_helper<T<A...>, T<B...>> = true;

template <typename T, typename U>
struct same_helper : false_t {};
template <typename T>
struct same_helper<T, T> : true_t {};

template <typename T>
struct type_identity {
  using type = T;
};

template <typename T>
auto try_add_lvalue_reference(int) -> type_identity<T &>;
template <typename T>
auto try_add_lvalue_reference(...) -> type_identity<T>;

template <typename>
struct is_function_helper : false_t {};
template <typename R, typename... Args>
struct is_function_helper<R(Args...)> : true_t {};
template <typename R, typename... Args>
struct is_function_helper<R(Args..., ...)> : true_t {};
template <typename T>
struct is_pointer_helper : false_t {};
template <typename T>
struct is_pointer_helper<T *> : true_t {};

template <typename T>
struct is_member_pointer_helper : false_t {};
template <typename T, typename U>
struct is_member_pointer_helper<T U::*> : true_t {};

template <typename T>
struct is_array_helper : false_t {};
template <typename T>
struct is_array_helper<T[]> : true_t {};
template <typename T, int N>
struct is_array_helper<T[N]> : true_t {};
}  // namespace internal

/**
 * @brief A struct used to denote a special template argument that means it's an
 * unused argument.
 */
struct unused {};

template <bool Condition, typename ConditionIsTrueType,
          typename ConditionIsFalseType>
struct type_select {
  using type = ConditionIsTrueType;
};

template <typename ConditionIsTrueType, typename ConditionIsFalseType>
struct type_select<false, ConditionIsTrueType, ConditionIsFalseType> {
  using type = ConditionIsFalseType;
};

/**
 * @brief A type alias that selects one of two type options based on a boolean
 * condition.
 *
 * This type alias is used to declare a type from one of two type options. The
 * result is based on the value of the `Condition` template parameter.
 *
 * Example usage:
 * @code
 *    using T = type_select_t<Condition, ChoiceAType, ChoiceBType>;
 * @endcode
 *
 * @tparam Condition A boolean value determining the selected type.
 * @tparam ConditionIsTrueType The type to select if `Condition` is true.
 * @tparam ConditionIsFalseType The type to select if `Condition` is false.
 */
template <bool Condition, typename ConditionIsTrueType,
          typename ConditionIsFalseType>
using type_select_t = typename type_select<Condition, ConditionIsTrueType,
                                           ConditionIsFalseType>::type;

template <typename T, typename = unused, typename = unused>
struct first_type_select {
  using type = T;
};

/**
 * @brief A type alias that unilaterally selects the first type.
 *
 * @tparam T The first type to select.
 * @tparam unused A placeholder unused type.
 * @tparam unused Another placeholder unused type.
 */
template <typename T, typename = unused, typename = unused>
using first_type_select_t = typename first_type_select<T>::type;

/**
 * @brief Concept to check if two types are the same.
 *
 * @tparam T The first type to compare.
 * @tparam U The second type to compare.
 */
template <typename T, typename U>
concept is_same = internal::same_helper<T, U>::value;

/**
 * @brief Concept to check if a type is the same as any of a list of given
 * types.
 *
 * @tparam T The type to compare against the list of types.
 * @tparam Types The list of types to compare with `T`.
 */
template <typename T, typename... Types>
concept is_same_to_one_of = (is_same<T, Types> || ...);

/**
 * @brief Concept to check if two types are the same, regardless of their
 * template parameters.
 *
 * This concept checks if two types have the same template but does not compare
 * their template parameters.
 *
 * Example usage:
 * @code
 *   is_same_template<array<int, 32>, array<float, 16>> // true
 * @endcode
 *
 * Note that it doesn't work if you mix types and typenames:
 * @code
 *   is_same_template<array, array<float, 16>> // false
 * @endcode
 *
 * @tparam T The first type to compare.
 * @tparam U The second type to compare.
 */
template <typename T, typename U>
concept is_same_template = internal::is_same_template_helper<T, U>;

/// @brief Concept to check if T has const-qualification.
template <typename T>
concept is_const = internal::is_const_helper_2<T>::value;

template <typename T>
struct remove_cv {
  using type = T;
};
template <typename T>
struct remove_cv<const T> {
  using type = T;
};
template <typename T>
struct remove_cv<const T[]> {
  using type = T[];
};
template <typename T, s64 N>
struct remove_cv<const T[N]> {
  using type = T[N];
};
template <typename T>
struct remove_cv<volatile T> {
  using type = T;
};
template <typename T>
struct remove_cv<volatile T[]> {
  using type = T[];
};
template <typename T, s64 N>
struct remove_cv<volatile T[N]> {
  using type = T[N];
};
template <typename T>
struct remove_cv<const volatile T> {
  using type = T;
};
template <typename T>
struct remove_cv<const volatile T[]> {
  using type = T[];
};
template <typename T, s64 N>
struct remove_cv<const volatile T[N]> {
  using type = T[N];
};

/// @brief Type alias to remove top-level const/volatile qualification from T.
template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

template <typename T>
struct remove_ref {
  using type = T;
};
template <typename T>
struct remove_ref<T &> {
  using type = T;
};
template <typename T>
struct remove_ref<T &&> {
  using type = T;
};

/**
 * @brief Type alias that removes top-level indirection by reference (if any)
 * from T.
 *
 * For a given type T, remove_reference_t<T&> is equivalent to T.
 */
template <typename T>
using remove_ref_t = typename remove_ref<T>::type;

template <typename T>
struct add_lvalue_reference
    : decltype(internal::try_add_lvalue_reference<T>(0)) {};

/**
 * @brief Type alias to add an l-value reference to T.
 *
 * Follows the rules (8.3.2 p6):
 * @code
 *      void + &  -> void
 *      T    + &  -> T&
 *      T&   + &  -> T&
 *      T&&  + &  -> T&
 * @endcode
 */
template <typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

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

/**
 * @brief Type alias to add an r-value reference to T.
 *
 * Follows the rules (8.3.2 p6):
 * @code
 *      void + &&  -> void
 *      T    + &&  -> T&&
 *      T&   + &&  -> T&
 *      T&&  + &&  -> T&&
 * @endcode
 */
template <typename T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

/**
 * @brief Function template that converts any type T to a reference type.
 *
 * This function template makes it possible to use member functions in decltype
 * expressions without specifying constructors. It has no use outside decltype
 * expressions.
 *
 * @tparam T The type to convert to a reference type.
 * @return A reference to T.
 */
template <typename T>
typename add_rvalue_reference<T>::type
declval() noexcept;  // It works with compiler magic I guess.

/// @brief Concept to check if T is an integral type.
template <typename T>
concept is_integral = numeric<T>::is_integral;

/// @brief Concept to check if T is a signed integral type.
template <typename T>
concept is_signed_integral = is_integral<T> && T(-1)
< T(0);

/// @brief Concept to check if T is an unsigned integral type.
template <typename T>
concept is_unsigned_integral = is_integral<T> && !is_signed_integral<T>;

/// @brief Concept to check if T is a floating point type.
template <typename T>
concept is_floating_point = is_same_to_one_of<remove_cv_t<T>, f32, f64>;

/// @brief Concept to check if T is an arithmetic type (integral or floating
/// point type).
template <typename T>
concept is_arithmetic = is_integral<T> || is_floating_point<T>;

/// @brief Concept to check if T is an enum type.
template <typename T>
concept is_enum = __is_enum(remove_cv_t<T>);

/// @brief Concept to check if T is a pointer to an object or a function, but
/// not to member objects/functions.
template <typename T>
concept is_function = internal::is_function_helper<T>::value;

/// @brief Tests whether T is a pointer to an object, to a function, but not to
/// member objects/functions.
template <typename T>
concept is_pointer = internal::is_pointer_helper<remove_cv_t<T>>::value;

/// @brief Concept to check if T is a pointer to a member object or a member
/// function.
template <typename T>
concept is_member_pointer =
    internal::is_member_pointer_helper<remove_cv_t<T>>::value;

/**
 * @brief Concept to check if T is a scalar type.
 *
 * A scalar is an integer type, a floating point type, an enum type, a pointer,
 * a member function pointer, or a null pointer type.
 */
template <typename T>
concept is_scalar = is_arithmetic<T> || is_enum<T> || is_pointer<T> ||
                    is_member_pointer<T> || is_same < remove_cv_t<T>,
decltype(nullptr) > ;

/**
 * @brief Concept to check if a type is convertible to another type.
 *
 * This concept checks if an object of type `From` can be implicitly converted
 * to an object of type `To` through a valid standard conversion sequence,
 * user-defined conversion, or an ellipsis conversion.
 *
 * @tparam From The type to be converted from.
 * @tparam To The type to be converted to.
 */
template <typename From, typename To>
concept is_convertible = __is_convertible_to(From, To);

/**
 * @brief Concept to check if a type can be constructed with the given set of
 * arguments.
 *
 * This concept checks if an object of type `T` can be constructed with a set of
 * arguments of types `Args...`. It checks if a constructor for type `T` exists
 * that accepts the provided argument types.
 *
 * @tparam T The type of the object to be constructed.
 * @tparam Args The types of arguments to be used for constructing the object.
 */
template <typename T, typename... Args>
concept is_constructible = __is_constructible(T, Args...);

template <typename T>
struct underlying_type {
  using type = __underlying_type(T);
};

/// @brief Alias template for the underlying type of an enum.
template <typename T>
using underlying_type_t = typename underlying_type<T>::type;

template <typename T>
struct remove_cvref {
  using type = remove_cv_t<remove_ref_t<T>>;
};

/**
 * @brief Alias template for the type obtained by removing top-level const
 * and/or volatile qualification and reference from the given type.
 */
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

/// @brief Concept to check if the decayed versions of "T" and "U" are the same
/// template type.
template <typename T, typename U>
concept is_same_template_decayed =
    is_same_template<remove_cvref_t<T>, remove_cvref_t<U>>;

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

/// @brief Removes the first level of pointers from T.
template <typename T>
using remove_pointer_t = typename remove_pointer<T>::type;

template <typename T>
struct add_pointer {
  using type = remove_ref_t<T> *;
};

// Adds a level of pointers to T
template <typename T>
using add_pointer_t = typename add_pointer<T>::type;

template <typename T>
concept is_array = internal::is_array_helper<T>::value;

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

/**
 * @brief The remove_extent transformation trait removes a dimension from an
 * array.
 *
 * This type trait is used to remove one level of array nesting from the input
 * type T. If the input type T is not an array, then remove_extent<T>::type is
 * equivalent to T. If the input type T is an array type T[N], then
 * remove_extent<T[N]>::type is equivalent to T. If the input type T is a
 * const-qualified array type const T[N], then remove_extent<const T[N]>::type
 * is equivalent to const T.
 *
 * For example, given a multi-dimensional array type T[M][N],
 * remove_extent<T[M][N]>::type is equivalent to T[N].
 */
template <typename T>
using remove_extent_t = typename remove_extent<T>::type;

template <typename T>
struct decay {
  using U = remove_ref_t<T>;

  using type = type_select_t<
      is_array<U>, remove_extent_t<U> *,
      type_select_t<is_function<U>, add_pointer_t<U>, remove_cv_t<U>>>;
};

/**
 * @brief Converts the type T to its decayed equivalent.
 *
 * This function template performs several type conversions to arrive at the
 * decayed equivalent of the input type T. These conversions include lvalue to
 * rvalue, array to pointer, function to pointer conversions, and removal of
 * const and volatile.
 *
 * The resulting type is the type conversion that's actually silently applied
 * by the compiler to all function arguments when passed by value.
 */
template <typename T>
using decay_t = typename decay<T>::type;

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
  using type =
      typename common_type<typename common_type<T, U>::type, V...>::type;
};

/**
 * @brief Computes the common type of a set of input types.
 * The common_type type trait computes the common type of a set of input types,
 * that is, the type to which every type in the parameter pack can be implicitly
 * converted to.
 *
 * It does so by using recursion and the common_type_helper struct to repeatedly
 * compute the common type of pairs of input types, until the common type of all
 * input types has been determined.
 */
template <typename... Ts>
using common_type_t = typename common_type<Ts...>::type;

/**
 * @brief Concept to check if the decayed versions of "T" and "U" are the same
 * basic type.
 *
 * This concept gets around the fact that `is_same` will treat `bool` and
 * `bool&` as different types.
 *
 * @tparam T The first type to compare.
 * @tparam U The second type to compare.
 */
template <typename T, typename U>
concept is_same_decayed = internal::same_helper<decay_t<T>, decay_t<U>>::value;

/**
 * @brief Safely converts between unrelated types that have a binary
 * equivalency.
 *
 * This approach is required by strictly conforming C++ compilers because
 * directly using a C or C++ cast between unrelated types is fraught with
 * the possibility of undefined runtime behavior due to type aliasing.
 *
 * @tparam DestType The destination type for the conversion.
 * @tparam SourceType The source type for the conversion.
 * @param sourceValue The source value to be converted.
 * @return The converted value of type DestType.
 *
 * Example usage:
 * @code
 *    f32 f = 1.234f;
 *    u32 br = bit_cast<u32>(f);
 * @endcode
 */
template <typename DestType, typename SourceType>
DestType bit_cast(SourceType const &sourceValue) {
  static_assert(sizeof(DestType) == sizeof(SourceType));
  return __builtin_bit_cast(DestType, sourceValue);
}

LSTD_END_NAMESPACE
