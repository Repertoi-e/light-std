#pragma once

#include "../memory/stack_array.h"
#include "vec_util.h"

LSTD_BEGIN_NAMESPACE

// Sets all elements of the vector to the same value.
//
// The "typename" before vec_info.. here is very important. It took me 2 hours of debugging. C++ is hell sometimes.
template <any_vec Vec, typename U>
    requires(types::is_convertible<U, typename vec_info<Vec>::T>)
void fill(Vec &lhs, U all) {
    if constexpr (!has_simd<Vec>) {
        For(lhs) it = (typename Vec::T) all;
    } else {
        using SimdT = decltype(Vec::Simd);
        lhs.Simd = SimdT::spread((typename Vec::T) all);
    }
}

// Calculates the scalar product (dot product) of the two arguments.
template <any_vec Vec>
auto dot(const Vec &lhs, const Vec &rhs) {
    if constexpr (!has_simd<Vec>) {
        auto sum = Vec::T(0);
        For(range(Vec::DIM)) sum += lhs.Data[it] * rhs.Data[it];
        return sum;
    } else {
        using SimdT = decltype(Vec::Simd);
        return SimdT::template dot<Vec::DIM>(lhs.Simd, rhs.Simd);
    }
}

// Returns true if the vector's length is too small for precise calculations (i.e. normalization).
// "Too small" means smaller than the square root of the smallest number representable by the underlying scalar.
// This value is ~10^-18 for floats and ~10^-154 for doubles.
template <any_vec Vec>
bool is_null_vector(const Vec &v) {
    using T = typename Vec::T;

    static constexpr T epsilon = T(1) / const_exp10<T>(abs(numeric_info<T>::min_exponent10) / 2);
    T length                   = len(v);
    return length < epsilon;
}

// Returns the squared length of the vector
template <any_vec Vec>
auto len_sq(const Vec &v) { return dot(v, v); }

// Returns the length of the vector
template <any_vec Vec>
auto len(const Vec &v) { return (typename Vec::T) sqrt((typename Vec::T) len_sq(v)); }

// Returns the length of the vector, avoids overflow and underflow, so it's more expensive
template <any_vec Vec>
auto len_precise(const Vec &v) {
    using T = typename Vec::T;

    T maxElement = abs(v[0]);
    for (s64 i     = 1; i < v.DIM; ++i)
        maxElement = max(maxElement, abs(v[i]));
    if (maxElement == T(0))
        return T(0);

    auto scaled = v / maxElement;
    return (T) sqrt(dot(scaled, scaled)) * maxElement;
}

// Returns the euclidean distance between to vectors
template <any_vec Vec, any_vec Other>
    requires(vec_info<Vec>::DIM == vec_info<Other>::DIM)
auto distance(const Vec &lhs, const Other &rhs) {
    return len(lhs - rhs);
}

// Makes a unit vector, but keeps direction
template <any_vec Vec>
Vec normalize(const Vec &v) {
    assert(!is_null_vector(v));
    return v / len(v);
}

// Checks if the vector is unit vector. There's some tolerance due to floating points
template <any_vec Vec>
bool is_normalized(const Vec &v) {
    using T = typename Vec::T;

    T n = len_sq(v);
    return T(0.9999) <= n && n <= T(1.0001);
}

// Makes a unit vector, but keeps direction. Leans towards (1,0,0...) for nullvectors, costs more
template <any_vec Vec>
Vec safe_normalize(const Vec &v) {
    using T = typename Vec::T;

    Vec vmod = v;
    vmod[0]  = abs(v[0]) > numeric_info<T>::denorm_min() ? v[0] : numeric_info<T>::denorm_min();
    T l      = len_precise(vmod);
    return vmod / l;
}

// Makes a unit vector, but keeps direction. Leans towards _degenerate_ for nullvectors, costs more.
// _degenerate_ Must be a unit vector.
template <any_vec Vec>
Vec safe_normalize(const Vec &v, const Vec &degenerate) {
    assert(is_normalized(degenerate));
    typename Vec::T length = len_precise(v);
    if (length == 0)
        return degenerate;
    return v / length;
}

// Returns the generalized cross-product in N dimensions.
// You must supply N-1 arguments of type _T_.
// The function returns the generalized cross product as defined by
// https://en.wikipedia.org/wiki/Cross_product#Multilinear_algebra.
template <any_vec Vec, typename... Args>
Vec cross(const Vec &head, Args &&...args);

// Returns the generalized cross-product in N dimensions.
// See https://en.wikipedia.org/wiki/Cross_product#Multilinear_algebra for definition.
template <any_vec Vec>
Vec cross(const stack_array<const Vec *, vec_info<Vec>::DIM - 1> &args);

// Returns the 2-dimensional cross product, which is a vector perpendicular to the argument
template <any_vec Vec>
    requires(vec_info<Vec>::DIM == 2)
Vec cross(const Vec &arg) {
    return Vec(-arg.y, arg.x);
}

// Returns the 2-dimensional cross product, which is a vector perpendicular to the argument
template <any_vec Vec>
    requires(vec_info<Vec>::DIM == 2)
Vec cross(const stack_array<const Vec *, 1> &arg) {
    return cross(*arg[0]);
}

// Returns the 3-dimensional cross-product
template <any_vec Vec>
    requires(vec_info<Vec>::DIM == 3)
Vec cross(const Vec &lhs, const Vec &rhs) {
    if constexpr (has_simd<Vec>) {
        return Vec(lhs.yzx) * Vec(rhs.zxy) - Vec(lhs.zxy) * Vec(rhs.yzx);
    } else {
        return Vec(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x);
    }
}

// Returns the 3-dimensional cross-product
template <any_vec Vec>
    requires(vec_info<Vec>::DIM == 3)
Vec cross(const stack_array<const Vec *, 2> &args) {
    return cross(*args[0], *args[1]);
}

// Returns the element-wise minimum of arguments
template <any_vec Vec>
always_inline Vec min(const Vec &lhs, const Vec &rhs) {
    Vec result;
    For(range(Vec::DIM)) result[it] = min(lhs[it], rhs[it]);
    return result;
}

// Returns the element-wise maximum of arguments
template <any_vec Vec>
always_inline Vec max(const Vec &lhs, const Vec &rhs) {
    Vec result;
    For(range(Vec::DIM)) result[it] = max(lhs[it], rhs[it]);
    return result;
}

// Clamps each vector value with specified bounds
template <any_vec Vec>
always_inline Vec clamp(const Vec &arg, typename vec_info<Vec>::T lower, typename vec_info<Vec>::T upper) {
    Vec result;
    For(range(Vec::DIM)) result[it] = clamp(arg[it], lower, upper);
    return result;
}

// Returns the element-wise natural log of the vector
template <any_vec Vec>
always_inline Vec ln(const Vec &vec) {
    Vec result;
    For(range(Vec::DIM)) result[it] = ln(vec[it]);
    return result;
}

// Returns the element-wise exp of the vector
template <any_vec Vec>
always_inline Vec exp(const Vec &vec) {
    Vec result;
    For(range(Vec::DIM)) result[it] = exp(vec[it]);
    return result;
}

// Returns the element-wise sqrt of the vector
template <any_vec Vec>
always_inline Vec sqrt(const Vec &vec) {
    Vec result;
    For(range(Vec::DIM)) result[it] = sqrt(vec[it]);
    return result;
}

// Returns the element-wise abs of the vector
template <any_vec Vec>
always_inline Vec abs(const Vec &vec) {
    Vec result;
    For(range(Vec::DIM)) result[it] = abs(vec[it]);
    return result;
}

// Returns the sum of the elements in the vector
template <any_vec Vec>
always_inline auto sum(const Vec &vec) {
    auto result = vec[0];
    For(range(1, Vec::DIM)) result += vec[it];
    return result;
}

// Returns the max of the elements in the vector
template <any_vec Vec>
always_inline auto max(const Vec &vec) {
    auto result = vec[0];
    For(range(1, Vec::DIM)) if (vec[it] > result) result = vec[it];
    return result;
}

// Returns the min of the elements in the vector
template <any_vec Vec>
always_inline auto min(const Vec &vec) {
    auto result = vec[0];
    For(range(1, Vec::DIM)) if (vec[it] < result) result = vec[it];
    return result;
}

LSTD_END_NAMESPACE

// We need this for the generalized cross product
#include "mat_func.h"

LSTD_BEGIN_NAMESPACE

template <any_vec Vec>
Vec cross(const stack_array<const Vec *, vec_info<Vec>::DIM - 1> &args) {
    Vec result;
    mat<typename Vec::T, Vec::DIM - 1, Vec::DIM - 1, false> d;

    // Calculate elements of result on-by-one
    s64 sign = 2 * (Vec::DIM % 2) - 1;
    for (s64 base = 0; base < Vec::DIM; ++base, sign *= -1) {
        // Fill up sub-matrix the determinant of which yields the coefficient of base-vector
        For_as(j, range(base)) {
            For_as(i, range(d.R)) { d(i, j) = (*args[i])[j]; }
        }

        For_as(j, range(base + 1, result.DIM)) {
            For_as(i, range(d.R)) { d(i, j - 1) = (*args[i])[j]; }
        }

        auto coef    = typename Vec::T(sign) * det(d);
        result[base] = coef;
    }
    return result;
}

template <any_vec Vec, typename... Args>
Vec cross(const Vec &head, Args &&...args) {
    static_assert(1 + sizeof...(args) == Vec::DIM - 1, "Number of arguments must be (Dim - 1).");

    stack_array<const Vec *, Vec::DIM - 1> vectors = {&head, &args...};
    return cross(vectors);
}

LSTD_END_NAMESPACE
