#pragma once

#include "transforms/identity.h"
#include "transforms/zero.h"

LSTD_BEGIN_NAMESPACE

// A utility that can do common operations with the QR decomposition,
//   i.e. solving equation systems
template <typename T, s64 R, s64 C, bool Packed>
struct decomposition_qr {
    using MatrixT = mat<T, R, C, Packed>;

    mat<T, R, R, Packed> Q;
    mat<T, R, C, Packed> R;
};

// Calculates the QR decomposition of the matrix using Householder transforms.
// The matrix must have R >= C. It's a full QR decomposition, not a thin one.
template <typename T, s64 R_, s64 C, bool Packed>
auto decompose_qr(mat<T, R_, C, Packed> m) {
    static_assert(R_ >= C);

    mat<T, R_, R_, Packed> Q;
    mat<T, R_, C, Packed> R;

    R = m;
    Q = identity();

    mat<T, R_, R_, Packed> Qi;
    vec<T, R_, Packed> u;
    mat<T, R_, 1, Packed> v;

    For_as(col, range(m.C)) {
        u = R.col(col);
        For(range(col)) u[it] = T(0);

        T alpha = sign(R(col, col)) * len_precise(u);
        u[col] -= alpha;
        T norm = len_precise(u);
        if (norm == 0) continue;

        u /= norm;
        v  = u;
        Qi = dot(T(-2) * v, ::T(v));

        For(range(Q.C)) Qi(it, it) += T(1);
        R = dot(Qi, R);
        Q = dot(Qi, Q);
    }
    Q = ::T(Q);
    return decomposition_qr<T, R_, C, Packed>{Q, R};
}

LSTD_END_NAMESPACE
