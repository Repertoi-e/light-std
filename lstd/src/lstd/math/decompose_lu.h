#pragma once

#include "vec.h"

LSTD_BEGIN_NAMESPACE

// A utility typename that can do common operations with the LU decomposition,
// i.e. solving equation systems
template <typename T, s64 Dim, bool Packed>
struct decomposition_lu {
    using MatrixT = mat<T, Dim, Dim, Packed>;

    // Lower triangular matrix, LU=P'A.
    MatrixT L;
    // Upper triangular matrix, LU=P'A.
    MatrixT U;

    // Solves the equation system Ax=b, that is LUx=b
    // If the equation is singular or the LU decomposition fails, garbage is returned.
    vec<f32, Dim, Packed> solve(const vec<T, Dim, Packed> &b) const { return solve_impl(L, U, b); }

    bool solvable() const {
        T prod = L(0, 0);
        T sum  = abs(prod);
        for (s64 i = 1; i < Dim; ++i) {
            prod *= L(i, i);
            sum += abs(L(i, i));
        }
        sum /= Dim;
        return abs(prod) / sum > T(1e-6);
    }

    static vec<f32, Dim, Packed> solve_impl(const MatrixT &L, const MatrixT &U, const vec<T, Dim, Packed> &b);
};

// A utility typename that can do common operations with the LUP decomposition,
// i.e. solving equation systems
template <typename T, s64 Dim, bool Packed>
struct decomposition_lup {
    using MatrixT = mat<T, Dim, Dim, Packed>;

    // Lower triangular matrix, LU=P'A.
    MatrixT L;
    // Upper triangular matrix, LU=P'A.
    MatrixT U;
    // Row permutations. LU=P'A, where P' is a matrix whose i-th row's P[i]-th element is one.
    vec<s64, Dim, false> P;

    // Solves the equation system Ax=b, that is LUx=Pb
    // If the equation is singular garbage is returned
    vec<f32, Dim, Packed> solve(const vec<T, Dim, Packed> &b) const;

    bool solvable() {
        T prod = L(0, 0);
        T sum  = abs(prod);
        for (s64 i = 1; i < Dim; ++i) {
            prod *= L(i, i);
            sum += abs(L(i, i));
        }
        sum /= Dim;
        return abs(prod) / sum > T(1e-6);
    }
};

template <typename T, s64 Dim, bool Packed>
auto decompose_lu(const mat<T, Dim, Dim, Packed> &m) {
    // From:
    // https://www.gamedev.net/resources/_/technical/math-and-physics/matrix-inversion-using-lu-decomposition-r3637
    mat<T, Dim, Dim, Packed> L, U;

    const auto &A   = m;
    constexpr s64 n = Dim;

    for (s64 i = 0; i < n; ++i) {
        for (s64 j = i + 1; j < n; ++j) {
            L(i, j) = 0;
        }
        for (s64 j = 0; j <= i; ++j) {
            U(i, j) = i == j;
        }
    }

    // Crout's algorithm
    for (s64 i = 0; i < n; ++i) {
        L(i, 0) = A(i, 0);
    }
    for (s64 j = 1; j < n; ++j) {
        U(0, j) = A(0, j) / L(0, 0);
    }

    for (s64 j = 1; j < n - 1; ++j) {
        for (s64 i = j; i < n; ++i) {
            T Lij = A(i, j);
            for (s64 k = 0; k <= j - 1; ++k) {
                Lij -= L(i, k) * U(k, j);
            }
            L(i, j) = Lij;
        }
        for (s64 k = j; k < n; ++k) {
            T Ujk = A(j, k);
            for (s64 i = 0; i <= j - 1; ++i) {
                Ujk -= L(j, i) * U(i, k);
            }
            Ujk /= L(j, j);
            U(j, k) = Ujk;
        }
    }

    L(n - 1, n - 1) = A(n - 1, n - 1);
    for (s64 k = 0; k < n - 1; ++k) {
        L(n - 1, n - 1) -= L(n - 1, k) * U(k, n - 1);
    }

    return decomposition_lu<T, Dim, Packed>{L, U};
}

// Implements LU decomposition with partial pivoting.
// Handles singular matrices as well.
// The parity of the permutation described with odd: 1, even: -1
template <typename T, s64 Dim, bool Packed>
auto decompose_lup(const mat<T, Dim, Dim, Packed> &m, s64 &parity) {
    mat<T, Dim, Dim, Packed> L, U;
    vec<s64, Dim, false> P;
    U = m;

    s64 n  = m.R;
    parity = 1;

    For(range(n)) P[it] = it;

    For_as(j, range(n)) {
        // Find largest pivot elements
        T p = 0;
        s64 largest;

        For_as(i, range(j, n)) {
            if (abs(U(i, j)) > p) {
                largest = i;
                p       = abs(U(i, j));
            }
        }
        if (p == 0) continue;

        // swap rows to move pivot to top row
        swap(P[j], P[largest]);
        parity *= j != largest ? -1 : 1;
        For(range(n)) swap(U(j, it), U(largest, it));

        // Do some magic
        For_as(i, range(j + 1, n)) {
            U(i, j) = U(i, j) / U(j, j);
            For_as(k, range(j + 1, n)) U(i, k) = U(i, k) - U(i, j) * U(j, k);
        }
    }

    // Copy elements to L
    For_as(j, range(n)) {
        For_as(i, range(j + 1, n)) {
            L(i, j) = U(i, j);
            U(i, j) = T(0);
            L(j, i) = T(0);
        }
    }

    For(range(n)) L(it, it) = 1;

    return decomposition_lup<T, Dim, Packed>{L, U, P};
}

// Implements LU decomposition with partial pivoting.
// Handles singular matrices as well.
template <typename T, s64 Dim, bool Packed>
auto decompose_lup(const mat<T, Dim, Dim, Packed> &m) {
    s64 ignore;
    return decompose_lup(m, ignore);
}

template <typename T, s64 Dim, bool Packed>
vec<f32, Dim, Packed> decomposition_lu<T, Dim, Packed>::solve_impl(const MatrixT &L,
                                                                   const MatrixT &U,
                                                                   const vec<T, Dim, Packed> &b) {
    // Matrix to do Gaussian elimination with
    mat<T, Dim, Dim + 1, Packed> E;

    // Solve Ld = b;
    E.get_view < Dim, Dim > (0, 0) = L;
    E.col(Dim)                     = b;

    for (s64 i = 0; i < Dim - 1; ++i) {
        for (s64 i2 = i + 1; i2 < Dim; ++i2) {
            E.Stripes[i] /= E(i, i);
            T coeff = E(i2, i);
            E.Stripes[i2] -= E.Stripes[i] * coeff;
        }
    }
    E(Dim - 1, Dim) /= E(Dim - 1, Dim - 1);
    // d is now the last column of E

    // Solve Ux = d
    E.get_view < Dim, Dim > (0, 0) = U;

    for (s64 i = Dim - 1; i > 0; --i) {
        for (s64 i2 = i - 1; i2 >= 0; --i2) {
            E.Stripes[i] /= E(i, i);
            T coeff = E(i2, i);
            E.Stripes[i2] -= E.Stripes[i] * coeff;
        }
    }
    E(0, Dim) /= E(0, 0);

    // x is now the last column of E
    return E.col(Dim);
}

template <typename T, s64 Dim, bool Packed>
vec<f32, Dim, Packed> decomposition_lup<T, Dim, Packed>::solve(const vec<T, Dim, Packed> &b) const {
    // Permute b
    vec<T, Dim, Packed> bp;
    For(range(P.DIM)) bp[it] = b[P[it]];

    return decomposition_lu<T, Dim, Packed>::solve_impl(L, U, bp);
}

LSTD_END_NAMESPACE
