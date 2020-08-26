#pragma once

#include "vec.h"

LSTD_BEGIN_NAMESPACE

//
// Concatenation
//

template <typename T, s64 Dim, bool Packed, typename U>
vec<T, Dim + 1, Packed> operator|(const vec<T, Dim, Packed> &lhs, U rhs) {
    vec<T, Dim + 1, Packed> result = {no_init};
    For(range(Dim)) result[it] = lhs[it];
    result[Dim] = rhs;
    return result;
}

template <typename T1, s64 Dim1, typename T2, s64 Dim2, bool Packed>
vec<T1, Dim1 + Dim2, Packed> operator|(const vec<T1, Dim1, Packed> &lhs, const vec<T2, Dim2, Packed> &rhs) {
    vec<T1, Dim1 + Dim2, Packed> result = {no_init};
    For(range(Dim1)) result[it] = lhs[it];
    For(range(Dim2)) result[Dim1 + it] = rhs[it];
    return result;
}

template <typename T, s64 Dim, bool Packed, typename U>
vec<T, Dim + 1, Packed> operator|(U lhs, const vec<T, Dim, Packed> &rhs) {
    vec<T, Dim + 1, Packed> result = {no_init};
    result(0) = lhs;
    For(range(Dim)) result[it + 1] = rhs[it];
    return result;
}

template <typename VectorData1, s64... Indices1, typename VectorData2, s64... Indices2>
auto operator|(const swizzle<VectorData1, Indices1...> &lhs, const swizzle<VectorData2, Indices2...> &rhs) {
    using TS1 = typename vec_info<VectorData1>::T;
    using TS2 = typename vec_info<VectorData2>::T;
    return vec<TS1, sizeof...(Indices1), false>(lhs) | vec<TS2, sizeof...(Indices2), false>(rhs);
}

template <typename VectorData1, s64... Indices1, typename T2, s64 Dim, bool Packed>
auto operator|(const swizzle<VectorData1, Indices1...> &lhs, const vec<T2, Dim, Packed> &rhs) {
    using TS = typename vec_info<VectorData1>::T;
    return vec<TS, sizeof...(Indices1), Packed>(lhs) | rhs;
}

template <typename VectorData1, s64... Indices1, typename T2, s64 Dim, bool Packed>
auto operator|(const vec<T2, Dim, Packed> &lhs, const swizzle<VectorData1, Indices1...> &rhs) {
    using TS = typename vec_info<VectorData1>::T;
    return lhs | vec<TS, sizeof...(Indices1), false>(rhs);
}

template <typename VectorData1, s64... Indices1, typename U>
auto operator|(const swizzle<VectorData1, Indices1...> &lhs, U rhs) {
    using TS = typename vec_info<VectorData1>::T;
    return vec<TS, sizeof...(Indices1), false>(lhs) | rhs;
}

template <typename VectorData1, s64... Indices1, typename U>
auto operator|(U lhs, const swizzle<VectorData1, Indices1...> &rhs) {
    using TS = typename vec_info<VectorData1>::T;
    return lhs | vec<TS, sizeof...(Indices1), false>(rhs);
}

//
// Arithmetic
//

// Elementwise (Hadamard) vector product
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> operator*(const vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        vec<T, Dim, Packed> result = {no_init};
        for (s64 i = 0; i < Dim; ++i) result[i] = lhs.Data[i] * rhs.Data[i];
        return result;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        return {vec<T, Dim, Packed>::FROM_SIMD, SimdT::mul(lhs.Simd, rhs.Simd)};
    }
}

// Elementwise vector division
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> operator/(const vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        vec<T, Dim, Packed> result = {no_init};
        for (s64 i = 0; i < Dim; ++i) result[i] = lhs.Data[i] / rhs.Data[i];
        return result;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        return {vec<T, Dim, Packed>::FROM_SIMD, SimdT::div(lhs.Simd, rhs.Simd)};
    }
}

// Elementwise vector addition
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> operator+(const vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        vec<T, Dim, Packed> result = {no_init};
        for (s64 i = 0; i < Dim; ++i) result[i] = lhs.Data[i] + rhs.Data[i];
        return result;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        return {vec<T, Dim, Packed>::FROM_SIMD, SimdT::add(lhs.Simd, rhs.Simd)};
    }
}
// Elementwise vector subtraction
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> operator-(const vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        vec<T, Dim, Packed> result = {no_init};
        for (s64 i = 0; i < Dim; ++i) result[i] = lhs.Data[i] - rhs.Data[i];
        return result;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        return {vec<T, Dim, Packed>::FROM_SIMD, SimdT::sub(lhs.Simd, rhs.Simd)};
    }
}

// Assignment

// Elementwise (Hadamard) vector product
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> &operator*=(vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        For(range(Dim)) lhs.Data[it] *= rhs.Data[it];
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        lhs.Simd = SimdT::mul(lhs.Simd, rhs.Simd);
    }
    return lhs;
}

// Elementwise vector division
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> &operator/=(vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        For(range(Dim)) lhs.Data[it] /= rhs.Data[it];
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        lhs.Simd = SimdT::div(lhs.Simd, rhs.Simd);
    }
    return lhs;
}

// Elementwise vector addition
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> &operator+=(vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        For(range(Dim)) lhs.Data[it] += rhs.Data[it];
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        lhs.Simd = SimdT::add(lhs.Simd, rhs.Simd);
    }
    return lhs;
}

// Elementwise vector subtraction
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> &operator-=(vec<T, Dim, Packed> &lhs, const vec<T, Dim, Packed> &rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        For(range(Dim)) lhs.Data[it] -= rhs.Data[it];
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        lhs.Simd = SimdT::sub(lhs.Simd, rhs.Simd);
    }
    return lhs;
}

// Scales the vector by _rhs_
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> &operator*=(vec<T, Dim, Packed> &lhs, U rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        For(range(Dim)) lhs.Data[it] *= rhs;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        lhs.Simd = SimdT::mul(lhs.Simd, (T) rhs);
    }
    return lhs;
}

// Scales the vector by 1/_rhs_
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> &operator/=(vec<T, Dim, Packed> &lhs, U rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        For(range(Dim)) lhs.Data[it] /= rhs;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        lhs.Simd = SimdT::div(lhs.Simd, (T) rhs);
    }
    return lhs;
}

// Adds _rhs_ to each element of the vector
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> &operator+=(vec<T, Dim, Packed> &lhs, U rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        For(range(Dim)) lhs.Data[it] += rhs;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        lhs.Simd = SimdT::add(lhs.Simd, (T) rhs);
    }
    return lhs;
}

// Subtracts _rhs_ from each element of the vector
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> &operator-=(vec<T, Dim, Packed> &lhs, U rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        For(range(Dim)) lhs.Data[it] -= rhs;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        lhs.Simd = SimdT::sub(lhs.Simd, (T) rhs);
    }
    return lhs;
}

// Scales the vector by _rhs_
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> operator*(const vec<T, Dim, Packed> &lhs, U rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        vec<T, Dim, Packed> copy(lhs);
        copy *= rhs;
        return copy;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        return {vec<T, Dim, Packed>::FROM_SIMD, SimdT::mul(lhs.Simd, (T) rhs)};
    }
}

// Scales the vector by 1/_rhs_
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> operator/(const vec<T, Dim, Packed> &lhs, U rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        vec<T, Dim, Packed> copy(lhs);
        copy /= rhs;
        return copy;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        return {vec<T, Dim, Packed>::FROM_SIMD, SimdT::div(lhs.Simd, (T) rhs)};
    }
}

// Adds _rhs_ to each element of the vector
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> operator+(const vec<T, Dim, Packed> &lhs, U rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        vec<T, Dim, Packed> copy(lhs);
        copy += rhs;
        return copy;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        return {vec<T, Dim, Packed>::FROM_SIMD, SimdT::add(lhs.Simd, (T) rhs)};
    }
}

// Subtracts _rhs_ from each element of the vector
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> operator-(const vec<T, Dim, Packed> &lhs, U rhs) {
    if constexpr (!has_simd<vec<T, Dim, Packed>>) {
        vec<T, Dim, Packed> copy(lhs);
        copy -= rhs;
        return copy;
    } else {
        using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
        return {vec<T, Dim, Packed>::FROM_SIMD, SimdT::sub(lhs.Simd, (T) rhs)};
    }
}

// Scales vector by _lhs_
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> operator*(U lhs, const vec<T, Dim, Packed> &rhs) {
    return rhs * lhs;
}

// Adds _lhs_ to all elements of the vector
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> operator+(U lhs, const vec<T, Dim, Packed> &rhs) {
    return rhs + lhs;
}

// Makes a vector with _lhs_ as all elements, then subtracts _rhs_ from it
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> operator-(U lhs, const vec<T, Dim, Packed> &rhs) {
    return vec<T, Dim, Packed>(lhs) - rhs;
}

// Makes a vector with _lhs_ as all elements, then divides it by _rhs_
template <typename T, s64 Dim, bool Packed, typename U>
requires(types::is_convertible_v<U, T>) inline vec<T, Dim, Packed> operator/(U lhs, const vec<T, Dim, Packed> &rhs) {
    vec<T, Dim, Packed> copy(lhs);
    copy /= rhs;
    return copy;
}

// Negates all elements of the vector
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> operator-(const vec<T, Dim, Packed> &arg) {
    return arg * T(-1);
}

// Optional plus sign, leaves the vector as is
template <typename T, s64 Dim, bool Packed>
inline vec<T, Dim, Packed> operator+(const vec<T, Dim, Packed> &arg) {
    return arg;
}

// Swizzles

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> operator*(const vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v * decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> operator/(const vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v / decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> operator+(const vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v + decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> operator-(const vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v - decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> operator*(const swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return decltype(v)(s) * v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> operator/(const swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return decltype(v)(s) / v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> operator+(const swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return decltype(v)(s) + v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> operator-(const swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return decltype(v)(s) - v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> &operator*=(vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v *= decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> &operator/=(vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v /= decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> &operator+=(vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v += decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) vec<T, Dim, Packed> &operator-=(vec<T, Dim, Packed> &v, const swizzle<VectorDataT, Indices...> &s) {
    return v -= decltype(v)(s);
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) swizzle<VectorDataT, Indices...> &operator*=(swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return s = decltype(v)(s) * v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) swizzle<VectorDataT, Indices...> &operator/=(swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return s = decltype(v)(s) / v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) swizzle<VectorDataT, Indices...> &operator+=(swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
    return s = decltype(v)(s) + v;
}

template <typename T, s64 Dim, bool Packed, typename VectorDataT, s64... Indices>
requires(Dim == sizeof...(Indices) && types::is_same_v<T, typename vec_info<VectorDataT>::T>) swizzle<VectorDataT, Indices...> &operator-=(swizzle<VectorDataT, Indices...> &s, const vec<T, Dim, Packed> &v) {
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

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto operator*(const swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return VectorT(lhs) * rhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto operator/(const swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return VectorT(lhs) / rhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto operator+(const swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return VectorT(lhs) + rhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto operator-(const swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return VectorT(lhs) - rhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto operator*(U lhs, const swizzle<VectorDataT, Indices...> &rhs) {
    return rhs * lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto operator/(U lhs, const swizzle<VectorDataT, Indices...> &rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return lhs / VectorT(rhs);
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto operator+(U lhs, const swizzle<VectorDataT, Indices...> &rhs) {
    return rhs + lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto operator-(U lhs, const swizzle<VectorDataT, Indices...> &rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    return lhs - VectorT(rhs);
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto &operator*=(swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    lhs = VectorT(lhs) * rhs;
    return lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto &operator/=(swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    lhs = VectorT(lhs) / rhs;
    return lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto &operator+=(swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    lhs = VectorT(lhs) + rhs;
    return lhs;
}

template <typename VectorDataT, s64... Indices, typename U>
requires(types::is_convertible_v<U, typename vec_info<VectorDataT>::T>) auto &operator-=(swizzle<VectorDataT, Indices...> &lhs, U rhs) {
    using VectorT = vec<typename vec_info<VectorDataT>::T, vec_info<VectorDataT>::DIM, vec_info<VectorDataT>::PACKED>;
    lhs = VectorT(lhs) - rhs;
    return lhs;
}

//
// Aproximation
//

template <typename T>
bool almost_equal(T d1, T d2, true_t) {
    if (abs(d1) < 1e-38 && abs(d2) < 1e-38) return true;
    if ((d1 == 0 && d2 < 1e-4) || (d2 == 0 && d1 < 1e-4)) return true;
    T scaler = (T) pow(T(10), floor(log10(abs(d1))));
    d1 /= scaler;
    d2 /= scaler;
    d1 *= T(1000.0);
    d2 *= T(1000.0);
    return round(d1) == round(d2);
}

// Specialization for int, complex and custom types: simple equality.
template <typename T>
bool almost_equal(T d1, T d2, false_t) {
    return d1 == d2;
}

// Check equivalence with tolerance.
template <typename T, typename U>
requires(!types::is_vec_v<T> && !types::is_mat_v<T> && !types::is_quat_v<T>) bool almost_equal(T d1, U d2) {
    using P = types::mat_mul_elem_t<T, U>;
    return almost_equal(P(d1), P(d2), integral_constant<bool, types::is_floating_point_v<P>>());
}

template <typename T1, typename T2, s64 Dim, bool Packed1, bool Packed2>
bool almost_equal(const vec<T1, Dim, Packed1> &lhs, const vec<T2, Dim, Packed2> &rhs) {
    bool eq = true;
    for (s64 i = 0; i < Dim; ++i) eq = eq && almost_equal(lhs[i], rhs[i]);
    return eq;
}

LSTD_END_NAMESPACE
