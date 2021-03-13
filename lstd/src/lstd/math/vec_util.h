#pragma once

#include "vec.h"

LSTD_BEGIN_NAMESPACE

template <typename Vec, typename U>
concept vec_same_type = types::is_same<typename vec_info<Vec>::T, U>;

template <typename Vec, typename Other>
concept vecs_same_types = types::is_same<typename vec_info<Vec>::T, typename vec_info<Other>::T>;

template <typename Vec, typename Other>
concept vecs_same_dim = vec_info<Vec>::DIM == vec_info<Other>::DIM;

template <typename Vec, s64 Dims>
using same_vec_with_extra_dims = vec<typename vec_info<Vec>::T, vec_info<Vec>::DIM + Dims, vec_info<Vec>::PACKED>;

// If either vector is packed the result is true
template <typename Vec, typename Other>
constexpr bool determine_packed = vec_info<Vec>::PACKED || vec_info<Other>::PACKED;

//
// Comparison:
//
// Vectors are just Data and Count really.
//
// :CodeReusability: ==, !=, <, <=, >, >=, compare_*, find_*, and has functions are automatically
// generated for vectors because we have marked them as array-like. Take a look at "array_like.h"
//
// They can also be compared against other array-likes.

//
// Aproximation:
//

// Specialization for floating point types
template <typename T>
bool almost_equal(T d1, T d2, types::true_t) {
    if (abs(d1) < 1e-38 && abs(d2) < 1e-38) return true;
    if ((d1 == 0 && d2 < 1e-4) || (d2 == 0 && d1 < 1e-4)) return true;
    T s = (T) Math_ExpB_flt32(T(10), Math_RoundDown_flt32(Math_Log10_flt32(abs(d1))));
    d1 /= s;
    d2 /= s;
    d1 *= T(1000.0);
    d2 *= T(1000.0);
    return Math_Round_flt32(d1) == Math_Round_flt32(d2);
}

// Specialization for int, complex and custom types: simple equality.
template <typename T>
bool almost_equal(T d1, T d2, types::false_t) {
    return d1 == d2;
}

// Check equivalence with tolerance.
template <typename T, typename U>
requires(!types::is_vec<T> && !types::is_mat<T> && !types::is_quat<T>) bool almost_equal(T d1, U d2) {
    using P = mat_mul_elem_t<T, U>;
    return almost_equal(P(d1), P(d2), types::integral_constant<bool, types::is_floating_point<P>>());
}

template <any_vec Vec, any_vec Other>
requires(vecs_same_dim<Vec, Other>) bool almost_equal(const Vec &lhs, const Other &rhs) {
    bool equal = true;
    For(range(Vec::DIM)) equal = equal && almost_equal(lhs[it], rhs[it]);
    return equal;
}

//
// Concatenation
//

template <any_vec Vec, typename U>
requires(vec_same_type<Vec, U>) auto operator|(const Vec &lhs, U rhs) {
    same_vec_with_extra_dims<Vec, 1> result;
    For(range(lhs.DIM)) result[it] = lhs[it];
    result[lhs.DIM] = rhs;
    return result;
}

template <any_vec Vec, typename U>
requires(vec_same_type<Vec, U>) auto operator|(U lhs, const Vec &rhs) {
    same_vec_with_extra_dims<Vec, 1> result;
    result[0] = lhs;
    For(range(rhs.DIM)) result[it + 1] = rhs[it];
    return result;
}

template <any_vec Vec, any_vec Other>
requires(vecs_same_types<Vec, Other>) auto operator|(const Vec &lhs, const Other &rhs) {
    same_vec_with_extra_dims<Vec, vec_info<Other>::Dims> result;
    For(range(lhs.DIM)) result[it] = lhs[it];
    For(range(rhs.DIM)) result[lhs.DIM + it] = rhs[it];
    return result;
}

template <any_vec Vec, s64... Indices1, any_vec Other, s64... Indices2>
requires(vecs_same_types<Vec, Other>) auto operator|(const swizzle<Vec, Indices1...> &lhs, const swizzle<Other, Indices2...> &rhs) {
    using T = typename vec_info<Vec>::T;
    using U = typename vec_info<Other>::T;

    constexpr bool PACKED = determine_packed<Vec, Other>;
    return vec<T, sizeof...(Indices1), PACKED>(lhs) | vec<U, sizeof...(Indices2), PACKED>(rhs);
}

template <any_vec Vec, s64... Indices1, any_vec Other>
requires(vecs_same_types<Vec, Other>) auto operator|(const swizzle<Vec, Indices1...> &lhs, const Other &rhs) {
    return vec<typename vec_info<Vec>::T, sizeof...(Indices1), determine_packed<Vec, Other>>(lhs) | rhs;
}

template <any_vec Vec, s64... Indices1, any_vec Other>
requires(vecs_same_types<Vec, Other>) auto operator|(const Vec &lhs, const swizzle<Other, Indices1...> &rhs) {
    return lhs | vec<typename vec_info<Other>::T, sizeof...(Indices1), determine_packed<Vec, Other>>(rhs);
}

template <any_vec Vec, s64... Indices1, typename U>
requires(vec_same_type<Vec, U>) auto operator|(const swizzle<Vec, Indices1...> &lhs, U rhs) {
    return vec<typename vec_info<Vec>::T, sizeof...(Indices1), vec_info<Vec>::PACKED>(lhs) | rhs;
}

template <typename VectorData1, s64... Indices1, typename U>
requires(vec_same_type<VectorData1, U>) auto operator|(U lhs, const swizzle<VectorData1, Indices1...> &rhs) {
    return lhs | vec<typename vec_info<VectorData1>::T, sizeof...(Indices1), vec_info<VectorData1>::PACKED>(rhs);
}

//
// Arithmetic
//

// Elementwise (Hadamard) vector product, division, addition and subtraction

#define ELEMENT_WISE_OPERATOR(op, simdName)                                   \
    template <any_vec Vec>                                                    \
    always_inline Vec operator##op(const Vec &lhs, const Vec &rhs) {          \
        if constexpr (!has_simd<Vec>) {                                       \
            Vec result;                                           \
            For(range(Vec::DIM)) result[it] = lhs.Data[it] op rhs.Data[it];   \
            return result;                                                    \
        } else {                                                              \
            using SimdT = decltype(Vec::Simd);                                \
            return {Vec::FROM_SIMD, SimdT::##simdName##(lhs.Simd, rhs.Simd)}; \
        }                                                                     \
    }

ELEMENT_WISE_OPERATOR(*, mul)
ELEMENT_WISE_OPERATOR(/, div)
ELEMENT_WISE_OPERATOR(+, add)
ELEMENT_WISE_OPERATOR(-, sub)
#undef ELEMENT_WISE_OPERATOR

// Elementwise (Hadamard) vector product, division, addition and subtraction but with assignment

#define ELEMENT_WISE_OPERATOR_ASSIGNMENT(op, simdName)          \
    template <any_vec Vec>                                      \
    always_inline Vec &operator##op(Vec &lhs, const Vec &rhs) { \
        if constexpr (!has_simd<Vec>) {                         \
            For(range(Vec::DIM)) lhs.Data[it] op rhs.Data[it];  \
        } else {                                                \
            using SimdT = decltype(Vec::Simd);                  \
            lhs.Simd = SimdT::##simdName##(lhs.Simd, rhs.Simd); \
        }                                                       \
        return lhs;                                             \
    }

ELEMENT_WISE_OPERATOR_ASSIGNMENT(*=, mul)
ELEMENT_WISE_OPERATOR_ASSIGNMENT(/=, div)
ELEMENT_WISE_OPERATOR_ASSIGNMENT(+=, add)
ELEMENT_WISE_OPERATOR_ASSIGNMENT(-=, sub)
#undef ELEMENT_WISE_OPERATOR_ASSIGNMENT

// * scales the vector by _rhs_, / scales the vector by 1/_rhs_, + and - add/subtract _rhs_ to/from each element, but with assignment

#define ELEMENT_WISE_SCALAR_OPERATOR_ASSIGNMENT(op, simdName)                                                        \
    template <any_vec Vec, typename U>                                                                               \
    requires(types::is_convertible<U, typename vec_info<Vec>::T>) always_inline Vec &operator##op(Vec &lhs, U rhs) { \
        if constexpr (!has_simd<Vec>) {                                                                              \
            For(range(Vec::DIM)) lhs.Data[it] op rhs;                                                                \
        } else {                                                                                                     \
            using SimdT = decltype(Vec::Simd);                                                                       \
            lhs.Simd = SimdT::##simdName##(lhs.Simd, Vec::T(rhs));                                                   \
        }                                                                                                            \
        return lhs;                                                                                                  \
    }

ELEMENT_WISE_SCALAR_OPERATOR_ASSIGNMENT(*=, mul);
ELEMENT_WISE_SCALAR_OPERATOR_ASSIGNMENT(/=, div);
ELEMENT_WISE_SCALAR_OPERATOR_ASSIGNMENT(+=, add);
ELEMENT_WISE_SCALAR_OPERATOR_ASSIGNMENT(-=, sub);
#undef ELEMENT_WISE_SCALAR_OPERATOR_ASSIGNMENT

// * scales the vector by _rhs_, / scales the vector by 1/_rhs_, + and - add/subtract _rhs_ to/from each element

#define ELEMENT_WISE_SCALAR_OPERATOR(op, simdName)                                                                        \
    template <any_vec Vec, typename U>                                                                                    \
    requires(types::is_convertible<U, typename vec_info<Vec>::T>) always_inline Vec operator##op(const Vec &lhs, U rhs) { \
        if constexpr (!has_simd<Vec>) {                                                                                   \
            Vec copy = lhs;                                                                                               \
            copy op## = rhs;                                                                                              \
            return copy;                                                                                                  \
        } else {                                                                                                          \
            using SimdT = decltype(Vec::Simd);                                                                            \
            return {Vec::FROM_SIMD, SimdT::##simdName##(lhs.Simd, Vec::T(rhs))};                                          \
        }                                                                                                                 \
    }
ELEMENT_WISE_SCALAR_OPERATOR(*, mul);
ELEMENT_WISE_SCALAR_OPERATOR(/, div);
ELEMENT_WISE_SCALAR_OPERATOR(+, add);
ELEMENT_WISE_SCALAR_OPERATOR(-, sub);
#undef ELEMENT_WISE_SCALAR_OPERATOR

// Scales vector by _lhs_
template <any_vec Vec, typename U>
requires(types::is_convertible<U, typename vec_info<Vec>::T>) always_inline Vec operator*(U lhs, const Vec &rhs) {
    return rhs * lhs;
}

// Adds _lhs_ to all elements of the vector
template <any_vec Vec, typename U>
requires(types::is_convertible<U, typename vec_info<Vec>::T>) always_inline Vec operator+(U lhs, const Vec &rhs) {
    return rhs + lhs;
}

// Makes a vector with _lhs_ as all elements, then subtracts _rhs_ from it
template <any_vec Vec, typename U>
requires(types::is_convertible<U, typename vec_info<Vec>::T>) always_inline Vec operator-(U lhs, const Vec &rhs) {
    return Vec(lhs) - rhs;
}

// Makes a vector with _lhs_ as all elements, then divides it by _rhs_
template <any_vec Vec, typename U>
requires(types::is_convertible<U, typename vec_info<Vec>::T>) always_inline Vec operator/(U lhs, const Vec &rhs) {
    auto result = Vec(lhs);
    result /= rhs;
    return result;
}

// Negates all elements of the vector
template <any_vec Vec>
always_inline Vec operator-(const Vec &arg) {
    return arg * Vec::T(-1);
}

// Optional plus sign, leaves the vector as is
template <any_vec Vec>
always_inline Vec operator+(const Vec &arg) {
    return arg;
}

//
// Swizzles...
//
template <typename Vec, typename Other, s64... Indices>
concept swizzle_and_vec_match = ((vec_info<Vec>::DIM == sizeof...(Indices)) && (types::is_same<typename vec_info<Vec>::T, typename vec_info<Other>::T>) );

#define SWIZZLE_OPERATOR(op)                                                                                                      \
    template <any_vec Vec, any_vec Other, s64... Indices>                                                                         \
    requires(swizzle_and_vec_match<Vec, Other, Indices...>) Vec operator##op(const Vec &v, const swizzle<Other, Indices...> &s) { \
        return v op Vec(s);                                                                                                       \
    }                                                                                                                             \
                                                                                                                                  \
    template <any_vec Vec, any_vec Other, s64... Indices>                                                                         \
    requires(swizzle_and_vec_match<Vec, Other, Indices...>) Vec operator##op(const swizzle<Other, Indices...> &s, const Vec &v) { \
        return Vec(s) op v;                                                                                                       \
    }

SWIZZLE_OPERATOR(*)
SWIZZLE_OPERATOR(/)
SWIZZLE_OPERATOR(+)
SWIZZLE_OPERATOR(-)
#undef SWIZZLE_OPERATOR

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> &operator*=(vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v *= decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> &operator/=(vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v /= decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> &operator+=(vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v += decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> &operator-=(vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v -= decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same<T, typename vec_info<VectorDataT>::T>) swizzle<VectorDataT, Indices...> &operator*=(swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return s = decltype(v)(s) * v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same<T, typename vec_info<VectorDataT>::T>) swizzle<VectorDataT, Indices...> &operator/=(swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return s = decltype(v)(s) / v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same<T, typename vec_info<VectorDataT>::T>) swizzle<VectorDataT, Indices...> &operator+=(swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return s = decltype(v)(s) + v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same<T, typename vec_info<VectorDataT>::T>) swizzle<VectorDataT, Indices...> &operator-=(swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return s = decltype(v)(s) - v;
}

template <typename VData1, s64... Indices1, typename VData2, s64... Indices2>
auto operator*(const swizzle<VData1, Indices1...> &s1, const swizzle<VData2, Indices2...> &s2) {
    using V1 = vec<typename vec_info<VData1>::T, vec_info<VData1>::DIM, vec_info<VData1>::PACKED>;
    using V2 = vec<typename vec_info<VData2>::T, vec_info<VData2>::DIM, vec_info<VData2>::PACKED>;
    return V1(s1) * V2(s2);
}

template <typename VData1, s64... Indices1, typename VData2, s64... Indices2>
auto operator/(const swizzle<VData1, Indices1...> &s1, const swizzle<VData2, Indices2...> &s2) {
    using V1 = vec<typename vec_info<VData1>::T, vec_info<VData1>::DIM, vec_info<VData1>::PACKED>;
    using V2 = vec<typename vec_info<VData2>::T, vec_info<VData2>::DIM, vec_info<VData2>::PACKED>;
    return V1(s1) / V2(s2);
}

template <typename VData1, s64... Indices1, typename VData2, s64... Indices2>
auto operator+(const swizzle<VData1, Indices1...> &s1, const swizzle<VData2, Indices2...> &s2) {
    using V1 = vec<typename vec_info<VData1>::T, vec_info<VData1>::DIM, vec_info<VData1>::PACKED>;
    using V2 = vec<typename vec_info<VData2>::T, vec_info<VData2>::DIM, vec_info<VData2>::PACKED>;
    return V1(s1) + V2(s2);
}

template <typename VData1, s64... Indices1, typename VData2, s64... Indices2>
auto operator-(const swizzle<VData1, Indices1...> &s1, const swizzle<VData2, Indices2...> &s2) {
    using V1 = vec<typename vec_info<VData1>::T, vec_info<VData1>::DIM, vec_info<VData1>::PACKED>;
    using V2 = vec<typename vec_info<VData2>::T, vec_info<VData2>::DIM, vec_info<VData2>::PACKED>;
    return V1(s1) - V2(s2);
}

template <typename VData1, s64... Indices1, typename VData2, s64... Indices2>
auto operator*=(swizzle<VData1, Indices1...> &s1, const swizzle<VData2, Indices2...> &s2) {
    return s1 = s1 * s2;
}

template <typename VData1, s64... Indices1, typename VData2, s64... Indices2>
auto operator/=(swizzle<VData1, Indices1...> &s1, const swizzle<VData2, Indices2...> &s2) {
    return s1 = s1 / s2;
}

template <typename VData1, s64... Indices1, typename VData2, s64... Indices2>
auto operator+=(swizzle<VData1, Indices1...> &s1, const swizzle<VData2, Indices2...> &s2) {
    return s1 = s1 + s2;
}

template <typename VData1, s64... Indices1, typename VData2, s64... Indices2>
auto operator-=(swizzle<VData1, Indices1...> &s1, const swizzle<VData2, Indices2...> &s2) {
    return s1 = s1 - s2;
}

template <typename U, typename VectorDataT>
concept u_to_vector_type = types::is_convertible<U, typename vec_info<VectorDataT>::T>;

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto operator*(const swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return VectorT(lhs) * rhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto operator/(const swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return VectorT(lhs) / rhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto operator+(const swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return VectorT(lhs) + rhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto operator-(const swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return VectorT(lhs) - rhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto operator*(U lhs, const swizzle<VectorDataT, Indices...> &rhs) {
    return rhs * lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto operator/(U lhs, const swizzle<VectorDataT, Indices...> &rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return lhs / VectorT(rhs);
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto operator+(U lhs, const swizzle<VectorDataT, Indices...> &rhs) {
    return rhs + lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto operator-(U lhs, const swizzle<VectorDataT, Indices...> &rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return lhs - VectorT(rhs);
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto &operator*=(swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    lhs = VectorT(lhs) * rhs;
    return lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto &operator/=(swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    lhs = VectorT(lhs) / rhs;
    return lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto &operator+=(swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    lhs = VectorT(lhs) + rhs;
    return lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires u_to_vector_type<U, VectorDataT> auto &operator-=(swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    lhs = VectorT(lhs) - rhs;
    return lhs;
}

LSTD_END_NAMESPACE
