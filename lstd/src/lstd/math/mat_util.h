#pragma once

#include "mat.h"

LSTD_BEGIN_NAMESPACE

//
// Arithmetic
//

template <typename T, typename T2, s64 R, s64 C, bool Packed>
inline auto operator*(const mat<T, R, C, Packed> &lhs, const mat<T2, R, C, Packed> &rhs) {
    mat<T, R, C, Packed> result;
    for (s64 i = 0; i < result.StripeCount; ++i) {
        result.Stripes[i] = lhs.Stripes[i] * rhs.Stripes[i];
    }
    return result;
}

template <typename T, typename T2, s64 R, s64 C, bool Packed>
inline auto operator/(const mat<T, R, C, Packed> &lhs, const mat<T2, R, C, Packed> &rhs) {
    mat<T, R, C, Packed> result;
    for (s64 i = 0; i < result.StripeCount; ++i) {
        result.Stripes[i] = lhs.Stripes[i] / rhs.Stripes[i];
    }
    return result;
}

template <typename T1, typename T2, s64 Dim, bool Packed>
inline mat<T1, Dim, Dim, Packed> &operator*=(mat<T1, Dim, Dim, Packed> &lhs, const mat<T2, Dim, Dim, Packed> &rhs) {
    lhs = lhs * rhs;
    return lhs;
}

template <typename T1, typename T2, s64 Dim, bool Packed>
inline mat<T1, Dim, Dim, Packed> &operator/=(mat<T1, Dim, Dim, Packed> &lhs, const mat<T2, Dim, Dim, Packed> &rhs) {
    lhs = lhs / rhs;
    return lhs;
}

namespace impl {
template <typename T, typename U, s64 R, s64 C, bool Packed, s64... StripeIndices>
inline auto small_add(const mat<T, R, C, Packed> &lhs, const mat<U, R, C, Packed> &rhs, integer_sequence<StripeIndices...>) {
    using V = mat_mul_elem_t<T, U>;
    using ResultT = mat<V, R, C, Packed>;
    return ResultT{ResultT::FromStripes, (lhs.Stripes[StripeIndices] + rhs.Stripes[StripeIndices])...};
}

template <typename T, typename U, s64 R, s64 C, bool Packed, s64... StripeIndices>
inline auto small_sub(const mat<T, R, C, Packed> &lhs, const mat<U, R, C, Packed> &rhs, integer_sequence<StripeIndices...>) {
    using V = mat_mul_elem_t<T, U>;
    using ResultT = mat<V, R, C, Packed>;
    return ResultT{ResultT::FromStripes, (lhs.Stripes[StripeIndices] - rhs.Stripes[StripeIndices])...};
}
}  // namespace impl

template <typename T, typename U, s64 R, s64 C, bool Packed>
inline auto operator+(const mat<T, R, C, Packed> &lhs, const mat<U, R, C, Packed> &rhs) {
    using V = mat_mul_elem_t<T, U>;

    if constexpr (R * C == 4) {
        mat<V, R, C, Packed> result;
        for (s64 i = 0; i < result.R; ++i) {
            for (s64 j = 0; j < result.C; ++j) {
                result(i, j) = lhs(i, j) + rhs(i, j);
            }
        }
        return result;
    } else if constexpr (R <= 4 && C <= 4) {
        return impl::small_add(lhs, rhs, make_integer_sequence<types::decay_t<decltype(lhs)>::StripeCount>{});
    } else {
        mat<V, R, C, Packed> result;
        for (s64 i = 0; i < result.StripeCount; ++i) {
            result.Stripes[i] = lhs.Stripes[i] + rhs.Stripes[i];
        }
        return result;
    }
}

template <any_mat Mat, typename U>
inline auto operator+(const Mat& lhs, U rhs) { return lhs + Mat(rhs); }

template <any_mat Mat, typename U>
inline auto operator+(U lhs, const Mat& rhs) { return Mat(lhs) + rhs; }

template <typename T, typename U, s64 R, s64 C, bool Packed>
inline auto operator-(const mat<T, R, C, Packed> &lhs, const mat<U, R, C, Packed> &rhs) {
    using V = mat_mul_elem_t<T, U>;

    if constexpr (R * C == 4) {
        mat<V, R, C, Packed> result;
        For_as(i, range(R)) For_as(j, range(C)) result(i, j) = lhs(i, j) - rhs(i, j);
        return result;
    } else if constexpr (R <= 4 && C <= 4) {
        return impl::small_sub(lhs, rhs, make_integer_sequence<types::decay_t<decltype(lhs)>::StripeCount>{});
    } else {
        mat<V, R, C, Packed> result;
        For(range(result.StripeCount)) result.Stripes[it] = lhs.Stripes[it] - rhs.Stripes[it];
        return result;
    }
}

template <any_mat Mat, typename U>
inline auto operator-(const Mat& lhs, U rhs) { return lhs - Mat(rhs); }

template <any_mat Mat, typename U>
inline auto operator-(U lhs, const Mat& rhs) { return Mat(lhs) - rhs; }

template <typename T, typename U, s64 R, s64 C, bool Packed>
inline mat<U, R, C, Packed> &operator+=(mat<T, R, C, Packed> &lhs, const mat<U, R, C, Packed> &rhs) {
    lhs = lhs + rhs;
    return lhs;
}

template <any_mat Mat, typename U>
inline auto operator+=(Mat& lhs, U rhs) { return lhs += Mat(rhs); }

template <typename T, typename U, s64 R, s64 C, bool Packed>
inline mat<U, R, C, Packed> &operator-=(mat<T, R, C, Packed> &lhs, const mat<U, R, C, Packed> &rhs) {
    lhs = lhs - rhs;
    return lhs;
}

template <any_mat Mat, typename U>
inline auto operator-=(Mat& lhs, U rhs) { return lhs -= Mat(rhs); }

// Scalar multiplication
template <typename T, s64 R, s64 C, bool Packed, typename U>
requires(types::is_convertible<U, T>) inline mat<T, R, C, Packed> &operator*=(mat<T, R, C, Packed> &m, U s) {
    For(range(m.StripeCount)) m.Stripes[it] *= s;
    return m;
}

// Divides all elements of the mat by scalar
template <typename T, s64 R, s64 C, bool Packed, typename U>
requires(types::is_convertible<U, T>) inline mat<T, R, C, Packed> &operator/=(mat<T, R, C, Packed> &m, U s) {
    m *= U(1) / s;
    return m;
}

template <typename T, s64 R, s64 C, bool Packed, typename U>
requires(types::is_convertible<U, T>) mat<T, R, C, Packed> operator*(const mat<T, R, C, Packed> &m, U s) {
    mat<T, R, C, Packed> copy(m);
    copy *= s;
    return copy;
}

template <typename T, s64 R, s64 C, bool Packed, typename U>
requires(types::is_convertible<U, T>) mat<T, R, C, Packed> operator/(const mat<T, R, C, Packed> &m, U s) {
    mat<T, R, C, Packed> copy(m);
    copy /= s;
    return copy;
}

template <typename T, s64 R, s64 C, bool Packed, typename U>
requires(types::is_convertible<U, T>) mat<T, R, C, Packed> operator*(U s, const mat<T, R, C, Packed> &m) { return m * s; }

template <typename T, s64 R, s64 C, bool Packed, typename U>
requires(types::is_convertible<U, T>) mat<T, R, C, Packed> operator/(U s, const mat<T, R, C, Packed> &m) {
    mat<T, R, C, Packed> result;
    for (s64 i = 0; i < mat<T, R, C, Packed>::StripeCount; ++i) {
        result.Stripes[i] = T(s) / m.Stripes[i];
    }
    return result;
}

template <typename T, s64 R, s64 C, bool Packed>
auto operator+(const mat<T, R, C, Packed> &m) {
    return mat<T, R, C, Packed>(m);
}

template <typename T, s64 R, s64 C, bool Packed>
auto operator-(const mat<T, R, C, Packed> &m) {
    return mat<T, R, C, Packed>(m) * T(-1);
}

//
// Comparison
//

template <s64 R, s64 C, typename T1, typename T2, bool Packed1, bool Packed2>
bool operator==(const mat<T1, R, C, Packed1> &lhs, const mat<T2, R, C, Packed2> &rhs) {
    bool equal = true;
    for (s64 i = 0; i < R; ++i) {
        for (s64 j = 0; j < C; ++j) {
            equal = equal && lhs(i, j) == rhs(i, j);
        }
    }
    return equal;
}

template <s64 R, s64 C, typename T1, typename T2, bool Packed1, bool Packed2>
bool operator!=(const mat<T1, R, C, Packed1> &lhs, const mat<T2, R, C, Packed2> &rhs) {
    return !(lhs == rhs);
}

//
// Cast
//

namespace impl {
template <typename MatrixDestT, typename MatrixSourceT>
struct reinterpret_compatible : types::false_t {};

template <typename T1, typename T2, s64 R, s64 C, bool Packed1, bool Packed2>
struct reinterpret_compatible<mat<T1, R, C, Packed1>, mat<T2, R, C, Packed2>> {
    static constexpr bool value = is_convertible<T2, T1>;
};
}  // namespace impl

// Changes the type, order and layout of the matrix, but the elements stay at the same place
template <typename MatrixDestT, typename MatrixSourceT>
requires(impl::reinterpret_compatible<MatrixDestT, MatrixSourceT>::value) MatrixDestT mat_reinterpret_cast(const MatrixSourceT &source) {
    MatrixDestT dest;
    For_as(i, range(source.R)) {
        For_as(j, range(source.C)) dest(i, j) = typename mat_info<MatrixDestT>::T(source(i, j));
    }
    return dest;
}

// @TODO: Function to convert to column major!
// Changes the type, order and layout of the matrix.
// The elements are transposed according to the change in order.
// template <typename MatrixDestT, typename MatrixSourceT>
// auto mat_representation_cast(const MatrixSourceT &source)
//     -> enable_if_t<impl::representation_compatible<MatrixDestT, MatrixSourceT>::value, MatrixDestT> {
//     MatrixDestT dest;
//     For_as(i, range(source.R)) {
//         For_as(j, range(source.C)) {
//             if constexpr (mat_info<MatrixDestT>::Order == mat_info<MatrixSourceT>::Order) {
//                 dest(i, j) = typename mat_info<MatrixDestT>::T(source(i, j));
//             } else {
//                 dest(j, i) = typename mat_info<MatrixDestT>::T(source(i, j));
//             }
//         }
//     }
//     return dest;
// }

LSTD_END_NAMESPACE
