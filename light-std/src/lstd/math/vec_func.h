#pragma once

#include "../intrin.h"
#include "../storage/stack_array.h"
#include "vec_util.h"

LSTD_BEGIN_NAMESPACE

// Returns true if the vector's length is too small for precise calculations (i.e. normalization).
// "Too small" means smaller than the square root of the smallest number representable by the underlying scalar.
// This value is ~10^-18 for floats and ~10^-154 for doubles.
template <typename T, s64 Dim, bool Packed>
bool is_null_vector(const vec<T, Dim, Packed> &v) {
    static constexpr T epsilon = T(1) / const_exp10<T>(const_abs(numeric_info<T>::min_exponent10) / 2);
    T length = len(v);
    return length < epsilon;
}

// Returns the squared length of the vector
template <typename T, s64 Dim, bool Packed>
T len_sq(const vec<T, Dim, Packed> &v) {
    return dot(v, v);
}

// Returns the length of the vector
template <typename T, s64 Dim, bool Packed>
T len(const vec<T, Dim, Packed> &v) {
    return (T) sqrt((T) len_sq(v));
}

// Returns the length of the vector, avoids overflow and underflow, so it's more expensive
template <typename T, s64 Dim, bool Packed>
T len_precise(const vec<T, Dim, Packed> &v) {
    T maxElement = abs(v[0]);
    for (s64 i = 1; i < v.Dim; ++i) maxElement = max(maxElement, abs(v[i]));
    if (maxElement == T(0)) return T(0);

    auto scaled = v / maxElement;
    return (T) sqrt(dot(scaled, scaled)) * maxElement;
}

// Returns the euclidean distance between to vectors
template <typename T, typename U, s64 Dim, bool Packed1, bool Packed2>
auto distance(const vec<T, Dim, Packed1> &lhs, const vec<U, Dim, Packed2> &rhs) {
    return len(lhs - rhs);
}

// Makes a unit vector, but keeps direction
template <typename T, s64 Dim, bool Packed>
vec<T, Dim, Packed> normalize(const vec<T, Dim, Packed> &v) {
    assert(!is_null_vector(v));
    T l = len(v);
    return v / l;
}

// Checks if the vector is unit vector. There's some tolerance due to floating points
template <typename T, s64 Dim, bool Packed>
bool is_normalized(const vec<T, Dim, Packed> &v) {
    T n = len_sq(v);
    return T(0.9999) <= n && n <= T(1.0001);
}

// Makes a unit vector, but keeps direction. Leans towards (1,0,0...) for nullvectors, costs more
template <typename T, s64 Dim, bool Packed>
vec<T, Dim, Packed> safe_normalize(const vec<T, Dim, Packed> &v) {
    vec<T, Dim, Packed> vmod = v;
    vmod[0] = abs(v[0]) > numeric_info<T>::denorm_min() ? v[0] : numeric_info<T>::denorm_min();
    T l = len_precise(vmod);
    return vmod / l;
}

// Makes a unit vector, but keeps direction. Leans towards _degenerate_ for nullvectors, costs more.
// _degenerate_ Must be a unit vector.
template <typename T, s64 Dim, bool Packed>
vec<T, Dim, Packed> safe_normalize(const vec<T, Dim, Packed> &v, const vec<T, Dim, Packed> &degenerate) {
    assert(is_normalized(degenerate));
    T length = len_precise(v);
    if (length == 0) return degenerate;
    return v / length;
}

// Sets all elements of the vector to the same value
template <typename T, s64 Dim, bool Packed, typename U>
enable_if_t<is_convertible_v<U, T>> fill(vec<T, Dim, Packed> &lhs, U all) {
    if constexpr (!has_simd_v<vec<T, Dim, Packed>>) {
        for (auto &v : lhs) v = (T) all;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        lhs.Simd = SimdT::spread((T) all);
    }
}

// Calculates the scalar product (dot product) of the two arguments
template <typename T, s64 Dim, bool Packed>
T dot(const vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    if constexpr (!has_simd_v<vec<T, Dim, Packed>>) {
        T sum = T(0);
        for (s64 i = 0; i < Dim; ++i) sum += lhs.Data[i] * rhs.Data[i];
        return sum;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        return SimdT::template dot<Dim>(lhs.Simd, rhs.Simd);
    }
}

// Returns the generalized cross-product in N dimensions.
// You must supply N-1 arguments of type _T_.
// The function returns the generalized cross product as defined by
// https://en.wikipedia.org/wiki/Cross_product#Multilinear_algebra.
template <typename T, s64 Dim, bool Packed, typename... Args>
auto cross(const vec<T, Dim, Packed> &head, Args &&... args) -> vec<T, Dim, Packed>;

// Returns the generalized cross-product in N dimensions.
// See https://en.wikipedia.org/wiki/Cross_product#Multilinear_algebra for definition.
template <typename T, s64 Dim, bool Packed>
auto cross(const stack_array<const vec<T, Dim, Packed> *, Dim - 1> &args) -> vec<T, Dim, Packed>;

// Returns the 2-dimensional cross prodct, which is a vector perpendicular to the argument
template <typename T, bool Packed>
vec<T, 2, Packed> cross(const vec<T, 2, Packed> &arg) {
    return vec<T, 2, Packed>(-arg.y, arg.x);
}
// Returns the 2-dimensional cross prodct, which is a vector perpendicular to the argument
template <typename T, bool Packed>
vec<T, 2, Packed> cross(const stack_array<const vec<T, 2, Packed> *, 1> &arg) {
    return cross(*(arg[0]));
}

// Returns the 3-dimensional cross-product
template <typename T, bool Packed>
vec<T, 3, Packed> cross(const vec<T, 3, Packed> &lhs, const vec<T, 3, Packed> &rhs) {
    using VecT = vec<T, 3, Packed>;
    if constexpr (has_simd_v<VecT>) {
        return VecT(lhs.yzx) * VecT(rhs.zxy) - VecT(lhs.zxy) * VecT(rhs.yzx);
    } else {
        return VecT(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x);
    }
}

// Returns the 3-dimensional cross-product
template <typename T, bool Packed>
vec<T, 3, Packed> cross(const stack_array<const vec<T, 3, Packed> *, 2> &args) {
    return cross(*(args[0]), *(args[1]));
}

// Returns the element-wise minimum of arguments
template <typename T, s64 Dim, bool Packed>
vec<T, Dim, Packed> min(const vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    vec<T, Dim, Packed> result = {no_init};
    for (s64 i = 0; i < lhs.Dim; ++i) result[i] = min(lhs[i], rhs[i]);
    return result;
}

// Returns the element-wise maximum of arguments
template <typename T, s64 Dim, bool Packed>
vec<T, Dim, Packed> max(const vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    vec<T, Dim, Packed> result = {no_init};
    for (s64 i = 0; i < lhs.Dim; ++i) result[i] = max(lhs[i], rhs[i]);
    return result;
}

#include "mat_func.h"

template <typename T, s64 Dim, bool Packed>
auto cross(const stack_array<const vec<T, Dim, Packed> *, Dim - 1> &args) -> vec<T, Dim, Packed> {
    vec<T, Dim, Packed> result = {no_init};
    mat<T, Dim - 1, Dim - 1, false> d = {no_init};

    // Calculate elements of result on-by-one
    s64 sign = 2 * (Dim % 2) - 1;
    for (s64 base = 0; base < result.Dim; ++base, sign *= -1) {
        // Fill up sub-matrix the determinant of which yields the coefficient of base-vector
        For_as(j, range(base)) {
            For_as(i, range(d.R)) { d(i, j) = (*(args[i]))[j]; }
        }

        For_as(j, range(base + 1, result.Dim)) {
            For_as(i, range(d.R)) { d(i, j - 1) = (*(args[i]))[j]; }
        }

        T coef = T(sign) * det(d);
        result[base] = coef;
    }
    return result;
}

template <typename T, s64 Dim, bool Packed, typename... Args>
auto cross(const vec<T, Dim, Packed> &head, Args &&... args) -> vec<T, Dim, Packed> {
    static_assert(1 + sizeof...(args) == Dim - 1, "Number of arguments must be (Dim - 1).");

    stack_array<const vec<T, Dim, Packed> *, Dim - 1> vectors = {&head, &args...};
    return cross(vectors);
}

LSTD_END_NAMESPACE
