#pragma once

#include "vec.h"

LSTD_BEGIN_NAMESPACE

// Allows you to do quaternion math and represent rotation in a compact way.
//
// If it's packed we disable padding - overalignment in arrays (disables SIMD optimization)
//
// These are plain mathematical quaternions, so expect the operations to work as mathematically defined.
// There are helper functions to represent rotation with quaternions.
template <typename T, bool Packed = false>
struct tquat {
    static constexpr bool SimdAccelerated = has_simd<vec<T, 4, Packed>>;

    union {
        struct {
            T s, i, j, k;
        };

        struct {
            T w, x, y, z;
        };

        vec<T, 4, Packed> Vec;
    };

    // :MathTypesNoInit By default we don't init (to save on performance) but you can call a constructor with a scalar value of 0 to zero-init.
    tquat() {
    }

    tquat(const tquat &rhs)
        : Vec(rhs.Vec) {
    }

    tquat(T scalar, T x, T y, T z)
        : w(scalar),
          x(x),
          y(y),
          z(z) {
    }

    // Note: This is not axis angle rotation
    tquat(T scalar, const vec<T, 3, true> &vector)
        : w(scalar),
          x(vector.x),
          y(vector.y),
          z(vector.z) {
    }

    // Note: This is not axis angle rotation
    tquat(T scalar, const vec<T, 3, false> &vector)
        : w(scalar),
          x(vector.x),
          y(vector.y),
          z(vector.z) {
    }

    explicit tquat(const vec<T, 3, true> &vector)
        : tquat(0, vector) {
    }

    explicit tquat(const vec<T, 3, false> &vector)
        : tquat(0, vector) {
    }

    template <typename U, bool P>
    tquat(const tquat<U, P> &rhs)
        : Vec(rhs.Vec) {
    }

    // Convert a rotation matrix to equivalent quaternion.
    // Matrix must be in SO(3).
    template <typename U, bool PackedA>
    explicit tquat(const mat<U, 3, 3, PackedA> &rhs) {
        from_mat(rhs);
    }

    // Convert a rotation matrix to equivalent quaternion.
    // Matrix must be in SO(3). Translation part is ignored.
    template <typename U, bool PackedA>
    explicit tquat(const mat<U, 4, 3, PackedA> &rhs) {
        from_mat(rhs);
    }

    // Convert a rotation matrix to equivalent quaternion.
    // Matrix must be in SO(3). Translation part is ignored.
    template <typename U, bool PackedA>
    explicit tquat(const mat<U, 4, 4, PackedA> &rhs) {
        from_mat(rhs);
    }

    explicit tquat(const vec<T, 4, false> &vec)
        : Vec(vec) {
    }

    tquat &operator=(const tquat &rhs) {
        Vec = rhs.Vec;
        return *this;
    }

    template <typename U, bool P>
    tquat &operator=(const tquat<U, P> &rhs) {
        Vec = rhs.Vec;
        return *this;
    }

    // Convert a rotation matrix to equivalent quaternion.
    // Matrix must be in SO(3).
    template <typename U, bool PackedA>
    tquat &operator=(const mat<U, 3, 3, PackedA> &rhs) {
        from_mat(rhs);
        return *this;
    }

    // Convert a rotation matrix to equivalent quaternion.
    // Matrix must be in SO(3). Translation part is ignored.
    template <typename U, bool PackedA>
    tquat &operator=(const mat<U, 4, 3, PackedA> &rhs) {
        from_mat(rhs);
        return *this;
    }

    // Convert a rotation matrix to equivalent quaternion.
    // Matrix must be in SO(3). Translation part is ignored.
    template <typename U, bool PackedA>
    tquat &operator=(const mat<U, 4, 4, PackedA> &rhs) {
        from_mat(rhs);
        return *this;
    }

    const T scalar_part() const { return s; }
    const vec<T, 3, Packed> vector_part() const { return {x, y, z}; }

    // Returns the angle of the rotation represented by quaternion.
    // Only valid for unit quaternions.
    const T angle() const { return (sign_bit(s) ? -1 : 1) * 2 * (T) acos(clamp(abs(s) / len(Vec), T(-1), T(1))); }

    // Returns the axis of rotation represented by quaternion.
    // Only valid for unit quaternions. Returns (1,0,0) for near 180 degree rotations.
    const vec<T, 3, Packed> axis() const {
        auto direction = vector_part();
        return safe_normalize(direction);
    }

    template <typename U, bool PackedA>
    explicit operator mat<U, 3, 3, PackedA>() const {
        return to_mat<U, 3, 3, PackedA>();
    }

    // Creates a rotation matrix equivalent to the quaternion.
    template <typename U, bool PackedA>
    explicit operator mat<U, 4, 3, PackedA>() const {
        return to_mat<U, 4, 3, PackedA>();
    }

    // Creates a rotation matrix equivalent to the quaternion.
    template <typename U, bool PackedA>
    explicit operator mat<U, 4, 4, PackedA>() const {
        return to_mat<U, 4, 4, PackedA>();
    }

    template <typename U, bool PackedA>
    explicit operator vec<U, 3, PackedA>() const {
        return {x, y, z};
    }

protected:
    template <typename U, s64 R, s64 C, bool PackedA>
    mat<U, R, C, PackedA> to_mat() const {
        assert(is_normalized(Vec));

        mat<U, R, C, PackedA> result;
        result(0, 0) = 1 - 2 * (j * j + k * k);
        result(0, 1) = 2 * (i * j - k * s);
        result(0, 2) = 2 * (i * k + j * s);
        result(1, 0) = 2 * (i * j + k * s);
        result(1, 1) = 1 - 2 * (i * i + k * k);
        result(1, 2) = 2 * (j * k - i * s);
        result(2, 0) = 2 * (i * k - j * s);
        result(2, 1) = 2 * (j * k + i * s);
        result(2, 2) = 1 - 2 * (i * i + j * j);

        For_as(j, range(result.Width)) { For_as(i, range(j < 3 ? 3 : 0, result.Height)) result(i, j) = T(j == i); }
        return result;
    }

    template <typename U, s64 R, s64 C, bool PackedA>
    void from_mat(const mat<U, R, C, PackedA> &m) {
        assert(is_rotation_mat_3d(m));

        w = (T) sqrt(1 + m(0, 0) + m(1, 1) + m(2, 2)) * T(0.5);

        T div = T(1) / (T(4) * w);
        x     = (m(2, 1) - m(1, 2)) * div;
        y     = (m(0, 2) - m(2, 0)) * div;
        z     = (m(1, 0) - m(0, 1)) * div;
    }
};

using quat = tquat<f32>;
using quat32 = tquat<f32>;
using quat64 = tquat<f64>;

LSTD_END_NAMESPACE
