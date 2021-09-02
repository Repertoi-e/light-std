#pragma once

#include "../mat_util.h"
#include "../quat_util.h"
#include "zero.h"

LSTD_BEGIN_NAMESPACE

struct identity_helper : non_copyable {
    identity_helper() {
    }

    template <typename T, s64 R, s64 C, bool Packed>
    operator mat<T, R, C, Packed>() const {
        mat<T, R, C, Packed> m = zero();
        For(range(min(R, C))) m(it, it) = T(1);
        return m;
    }

    template <typename T, bool Packed>
    operator tquat<T, Packed>() const {
        return tquat<T, Packed>{1, 0, 0, 0};
    }
};

// Creates an identity matrix or identity quaternion.
// If the matrix is not square, it will look like a truncated larger square identity matrix.
//
// Example:
//    mat4 transform   = identity();
//    quat orientation = identity();
inline auto identity() { return identity_helper{}; }

LSTD_END_NAMESPACE
