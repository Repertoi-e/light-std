#pragma once

#include "quat.h"
#include "vec_util.h"

LSTD_BEGIN_NAMESPACE

//
// Comparison
//

// Check exact equality of coefficients
template <typename T, bool Packed>
bool operator==(const tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    return lhs.Vec == rhs.Vec;
}

// Check exact unequality of coefficients
template <typename T, bool Packed>
bool operator!=(const tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    return !(lhs == rhs);
}

//
// Arithmetic
//

template <typename T, bool Packed>
tquat<T, Packed> &operator+=(tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    lhs.Vec += rhs.Vec;
    return lhs;
}

// Helpers to write quaternion in paper-style such as (1 + 2_i + 3_j + 4_k). Slow performance, be careful.
template <typename T, bool Packed>
tquat<T, Packed> &operator-=(tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    lhs.Vec -= rhs.Vec;
    return lhs;
}

// DOES ELEMENT-WISE MULTIPLICATION (API CONSISTENT WITH MATRIX *)
template <typename T, bool Packed>
tquat<T, Packed> &operator*=(tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    lhs.Vec *= rhs.Vec;
    return lhs;
}

template <typename T, bool Packed>
tquat<T, Packed> &operator*=(tquat<T, Packed> &lhs, T s) {
    lhs.Vec *= s;
    return lhs;
}

template <typename T, bool Packed>
tquat<T, Packed> &operator/=(tquat<T, Packed> &lhs, T s) {
    lhs *= T(1) / s;
    return lhs;
}

template <typename T, bool Packed>
tquat<T, Packed> operator+(const tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    tquat<T, Packed> copy(lhs);
    copy += rhs;
    return copy;
}

template <typename T, bool Packed>
tquat<T, Packed> operator-(const tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    tquat<T, Packed> copy(lhs);
    copy -= rhs;
    return copy;
}

// DOES ELEMENT-WISE MULTIPLICATION (API CONSISTENT WITH MATRIX *)
template <typename T, bool Packed>
tquat<T, Packed> operator*(const tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    tquat<T, Packed> copy(lhs);
    copy *= rhs;
    return copy;
}

template <typename T, bool Packed>
tquat<T, Packed> operator*(const tquat<T, Packed> &lhs, T s) {
    tquat<T, Packed> copy(lhs);
    copy *= s;
    return copy;
}

template <typename T, bool Packed>
tquat<T, Packed> operator/(const tquat<T, Packed> &lhs, T s) {
    tquat<T, Packed> copy(lhs);
    copy /= s;
    return copy;
}

template <typename T, bool Packed>
tquat<T, Packed> operator+(const tquat<T, Packed> &arg) {
    return arg;
}

template <typename T, bool Packed>
tquat<T, Packed> operator-(const tquat<T, Packed> &arg) {
    return tquat(-arg.Vec);
}

// @Cleanup Combine with * above. Better yet, remove * from the math library because it's ambigious (element wise or dot product???????)
// Multiplies all coefficients of the quaternion by _s_
template <typename T, bool Packed, typename U>
requires(!types::is_same<U, tquat<T, Packed>>) tquat<T, Packed> operator*(U s, const tquat<T, Packed> &rhs) {
    return rhs * s;
}

// Divides all coefficients of the quaternion by _s_
template <typename T, bool Packed, typename U>
requires(!types::is_same<U, tquat<T, Packed>>) tquat<T, Packed> operator/(U s, const tquat<T, Packed> &rhs) {
    return rhs / s;
}

// Adds a real to the real part of the quaternion
template <typename T, bool Packed, typename U>
requires(!types::is_quat<U>::value) tquat<T, Packed> operator+(const U &lhs, const tquat<T, Packed> &rhs) {
    return tquat<T, Packed>(rhs.w + lhs, rhs.x, rhs.y, rhs.z);
}

//
// Approximation
//

template <typename T, bool Packed1, bool Packed2>
bool almost_equal(const tquat<T, Packed1> &lhs, const tquat<T, Packed2> &rhs) {
    bool eq = true;
    For(range(4)) eq = eq && almost_equal(lhs.Vec[it], rhs.Vec[it]);
    return eq;
}

LSTD_END_NAMESPACE
