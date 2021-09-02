#pragma once

#include "quat_util.h"

LSTD_BEGIN_NAMESPACE

namespace impl {
template <typename T, bool Packed>
    requires(!tquat<T, Packed>::SimdAccelerated)
tquat<T, Packed> product(const tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    tquat<T, Packed> result;
    result.w = lhs.s * rhs.s - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
    result.x = lhs.s * rhs.x + lhs.x * rhs.s + lhs.y * rhs.z - lhs.z * rhs.y;
    result.y = lhs.s * rhs.y - lhs.x * rhs.z + lhs.y * rhs.s + lhs.z * rhs.x;
    result.z = lhs.s * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.s;
    return result;
}

template <typename T, bool Packed>
    requires(tquat<T, Packed>::SimdAccelerated)
tquat<T, Packed> product(const tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    tquat<T, Packed> result;
    using SimdT = simd<T, 4>;

    SimdT dabc = lhs.Vec.Simd;
    SimdT wxyz = rhs.Vec.Simd;
    SimdT alternate;
    auto *v = (T *) &alternate.reg;
    v[0]    = -1;
    v[1]    = 1;
    v[2]    = -1;
    v[3]    = 1;

    // [ 3, 2, 1, 0 ]
    // [ 0, 3, 2, 1 ]
    SimdT t0 = SimdT::template shuffle<0, 0, 0, 0>(dabc);
    SimdT t1 = SimdT::template shuffle<3, 0, 1, 2>(wxyz);

    SimdT t2 = SimdT::template shuffle<1, 1, 1, 1>(dabc);
    SimdT t3 = SimdT::template shuffle<2, 1, 0, 3>(wxyz);

    SimdT t4 = SimdT::template shuffle<2, 2, 2, 2>(dabc);
    SimdT t5 = SimdT::template shuffle<3, 1, 0, 2>(wxyz);

    SimdT m0 = SimdT::mul(t0, t1);
    SimdT m1 = SimdT::mul(t2, t3);
    SimdT m2 = SimdT::mul(t4, t5);

    SimdT t6 = SimdT::template shuffle<3, 3, 3, 3>(dabc);
    SimdT t7 = SimdT::template shuffle<0, 3, 1, 2>(wxyz);

    SimdT m3 = SimdT::mul(t6, t7);

    SimdT e = SimdT::add(m0, SimdT::mul(alternate, m1));
    e       = SimdT::template shuffle<1, 3, 0, 2>(e);
    e       = SimdT::add(e, SimdT::mul(alternate, m2));
    e       = SimdT::template shuffle<2, 0, 1, 3>(e);
    e       = SimdT::add(e, SimdT::mul(alternate, m3));
    e       = SimdT::template shuffle<3, 1, 0, 2>(e);

    result.Vec.Simd = e;
    return result;
}
} // namespace impl

// Multiplies two quaterions (normal operator * does element wise!)
template <typename T, bool Packed>
tquat<T, Packed> qmul(const tquat<T, Packed> &lhs, const tquat<T, Packed> &rhs) {
    return impl::product(lhs, rhs);
}

// Rotates (and scales) vector by quaternion
template <typename T, bool QPacked, bool PackedA>
vec<T, 3, PackedA> rotate_vec(const vec<T, 3, PackedA> &v, const tquat<T, QPacked> &q) {
    // Sandwich product
    return vec<T, 3, PackedA>(qmul(qmul(q, tquat<T, QPacked>(v)), inverse(q)));
}

// The euclidean length of the vector of the 4 elements of the quaternion
template <typename T, bool Packed>
T abs(const tquat<T, Packed> &q) {
    return len(q.Vec);
}

// Negates the imaginary values of the quaternion
template <typename T, bool Packed>
tquat<T, Packed> conjugate(const tquat<T, Packed> &q) {
    return tquat<T, Packed>{q.Vec * vec<T, 4, Packed>{T(1), T(-1), T(-1), T(-1)}};
}

// Natural quaternion exponentiation
template <typename T, bool Packed>
tquat<T, Packed> exp(const tquat<T, Packed> &q) {
    auto a = q.scalar_part();
    auto v = q.vector_part();
    T mag  = len(v);
    T es   = (T) ::exp(a);

    tquat<T, Packed> ret = {(T) cos(mag), v * ((T) sin(mag) / mag)};
    ret *= es;

    return ret;
}

// Natural quaternion logarithm
template <typename T, bool Packed>
tquat<T, Packed> ln(const tquat<T, Packed> &q) {
    auto magq = len(q);
    auto vn   = normalize(q.vector_part());
    return {(T) log(magq), vn * (T) acos(q.s / magq)};
}

// Raises _q_ to the power of _a_
template <typename T, bool Packed>
tquat<T, Packed> pow(const tquat<T, Packed> &q, T a) {
    return exp(a * ln(q));
}

// Returns the square of the absolute value.
// Just like complex numbers, it's the square of the length of the vector formed by the coefficients.
template <typename T, bool Packed>
T len_sq(const tquat<T, Packed> &q) {
    return len_sq(q.Vec);
}

// Returns the absolute value of the quaternion.
// Just like complex numbers, it's the length of the vector formed by the coefficients.
template <typename T, bool Packed>
T len(const tquat<T, Packed> &q) {
    return abs(q);
}

// Returns the unit quaternion of the same direction. Does not change this object.
template <typename T, bool Packed>
tquat<T, Packed> normalize(const tquat<T, Packed> &q) {
    return tquat<T, Packed>{normalize(q.Vec)};
}

// Returns the quaternion of opposite rotation
template <typename T, bool Packed>
tquat<T, Packed> inverse(const tquat<T, Packed> &q) {
    return conjugate(q);
}

// Check if the quaternion is a unit quaternion, with some tolerance for floats
template <typename T, bool Packed>
bool is_normalized(const tquat<T, Packed> &q) {
    return is_normalized(q.Vec);
}

template <typename T, bool Packed>
vec<T, 3, Packed> to_euler_angles(const tquat<T, Packed> &q) {
    assert(is_normalized(q.Vec));
    vec<T, 3, Packed> result;

    // Roll/X
    double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    result.x         = (T) atan2(sinr_cosp, cosr_cosp);

    // Pitch/Y
    double sinp = 2 * (q.w * q.y - q.z * q.x);
    if (abs(sinp) >= 1) {
        result.y = TAU / 4 * sign_no_zero(sinp); // Use 90 degrees if out of range
    } else {
        result.y = (T) asin(sinp);
    }

    // Yaw/Z
    double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    result.z         = (T) atan2(siny_cosp, cosy_cosp);

    return result;
}

LSTD_END_NAMESPACE
