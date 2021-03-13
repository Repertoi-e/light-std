#pragma once

#include "mat_util.h"

LSTD_BEGIN_NAMESPACE

namespace impl {
template <typename T, typename U, s64 R1, s64 Match, s64 C2, bool Packed, s64... MatchIndices>
inline auto small_product_row_rr(const mat<T, R1, Match, Packed> &lhs, const mat<U, Match, C2, Packed> &rhs, s64 row, integer_sequence<MatchIndices...>) {
    return (... + (rhs.Stripes[MatchIndices] * lhs(row, MatchIndices)));
}

template <typename T, typename U, s64 R1, s64 Match, s64 C2, bool Packed, s64... RowIndices>
inline auto small_product_rr(const mat<T, R1, Match, Packed> &lhs, const mat<U, Match, C2, Packed> &rhs, integer_sequence<RowIndices...>) {
    using V = mat_mul_elem_t<T, U>;
    using ResultT = mat<V, R1, C2, Packed>;
    return ResultT{ResultT::FromStripes, small_product_row_rr(lhs, rhs, RowIndices, make_integer_sequence<Match>{})...};
}
}  // namespace impl

template <typename T, typename U, s64 R1, s64 Match, s64 C2, bool Packed>
inline mat<T, R1, C2, Packed> dot(const mat<T, R1, Match, Packed> &lhs, const mat<U, Match, C2, Packed> &rhs) {
    if constexpr (R1 == 4 && Match == 4 && C2 == 4 && types::is_same<T, f32> && types::is_same<U, f32>) {
        using V = mat_mul_elem_t<T, U>;
        mat<V, 4, 4, Packed> result;

        __m128 row[4], sum[4];
        for (s32 i = 0; i < 4; i++) row[i] = _mm_load_ps((f32 *) &rhs.Stripes[i]);
        for (s32 i = 0; i < 4; i++) {
            sum[i] = _mm_setzero_ps();
            for (int j = 0; j < 4; j++) {
                sum[i] = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(lhs(i, j)), row[j]), sum[i]);
            }
        }
        for (s32 i = 0; i < 4; i++) _mm_store_ps((f32 *) &result.Stripes[i], sum[i]);
        return result;
    } else {
        if constexpr (R1 <= 4 && Match <= 4 && C2 <= 4) {
            return impl::small_product_rr(lhs, rhs, make_integer_sequence<R1>{});
        } else {
            using V = mat_mul_elem_t<T, U>;
            mat<V, R1, C2, Packed> result;
            for (s64 i = 0; i < R1; ++i) {
                result.Stripes[i] = rhs.Stripes[0] * lhs(i, 0);
            }
            for (s64 i = 0; i < R1; ++i) {
                for (s64 j = 1; j < Match; ++j) {
                    result.Stripes[i] += rhs.Stripes[j] * lhs(i, j);
                }
            }
            return result;
        }
    }
}

// v*M
template <typename Vt, typename Mt, s64 Vd, s64 Mcol, bool Packed>
inline auto dot(const vec<Vt, Vd, Packed> &v, const mat<Mt, Vd, Mcol, Packed> &mat) {
    using Rt = mat_mul_elem_t<Vt, Mt>;
    vec<Rt, Mcol, Packed> result = v[0] * mat.Stripes[0];
    For(range(1, Vd)) result += v[it] * mat.Stripes[it];
    return result;
}

// (v|1)*M
template <typename Vt, typename Mt, s64 Vd, bool Packed>
inline auto dot(const vec<Vt, Vd, Packed> &v, const mat<Mt, Vd + 1, Vd, Packed> &mat) {
    return dot((v | Vt(1)), mat);
}

template <typename Vt, typename Mt, s64 Vd, bool Packed>
inline auto dot(const vec<Vt, Vd, Packed> &v, const mat<Mt, Vd + 1, Vd + 1, Packed> &mat) {
    using Rt = mat_mul_elem_t<Vt, Mt>;
    auto result = dot((v | Vt(1)), mat);
    result /= result[-1];
    return vec<Rt, Vd, Packed>(result);
}

// Returns the trace (sum of diagonal elements) of the matrix
template <typename T, s64 Dim, bool Packed>
T trace(const mat<T, Dim, Dim, Packed> &m) {
    T sum = m(0, 0);
    For(range(1, Dim)) sum += m(it, it);
    return sum;
}

// Returns the determinant of a 2x2 matrix
template <typename T, bool Packed>
T det(const mat<T, 2, 2, Packed> &m) {
    return m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1);
}

// Returns the determinant of a 3x3matrix
template <typename T, bool Packed>
T det(const mat<T, 3, 3, Packed> &m) {
    using Vec3 = vec<T, 3, false>;

    Vec3 r0_zyx = m.Stripes[0].zyx;
    Vec3 r1_xzy = m.Stripes[1].xzy;
    Vec3 r1_yxz = m.Stripes[1].yxz;
    Vec3 r2_yxz = m.Stripes[2].yxz;
    Vec3 r2_xzy = m.Stripes[2].xzy;

    T det = dot(r0_zyx, r1_xzy * r2_yxz - r1_yxz * r2_xzy);

    return det;
}

// Returns the determinant of a 4x4matrix
template <typename T, bool Packed>
T det(const mat<T, 4, 4, Packed> &m) {
    using Vec3 = vec<T, 3, false>;
    using Vec4 = vec<T, 4, false>;

    Vec4 evenPair = {1, -1, -1, 1};
    Vec4 oddPair = {-1, 1, 1, -1};

    const Vec4 &r0 = m.Stripes[0];
    const Vec4 &r1 = m.Stripes[1];
    const Vec4 &r2 = m.Stripes[2];
    const Vec4 &r3 = m.Stripes[3];

    Vec4 r2_zwzw = r2.zwzw;
    Vec4 r0_yyxx = r0.yyxx;
    Vec4 r1_wwxy = r1.wwxy;
    Vec4 r2_xyzz = r2.xyzz;
    Vec4 r3_wwww = r3.wwww;
    Vec4 r1_zzxy = r1.zzxy;
    Vec4 r0_yxyx = r0.yxyx;
    Vec4 r3_xxyy = r3.xxyy;
    Vec4 r1_wzwz = r1.wzwz;
    Vec4 r2_xyww = r2.xyww;
    Vec4 r3_zzzz = r3.zzzz;

    Vec3 r2_yxz = r2.yxz;
    Vec3 r3_xzy = r3.xzy;
    Vec3 r2_xzy = r2.xzy;
    Vec3 r3_yxz = r3.yxz;
    Vec3 r2_yxw = r2.yxw;
    Vec3 r1_zyx = r1.zyx;
    Vec3 r3_yxw = r3.yxw;
    Vec3 r2_xwy = r2.xwy;
    Vec3 r3_xwy = r3.xwy;
    Vec3 r1_wyx = r1.wyx;
    T r0_w = r0.w;
    T r0_z = r0.z;

    T det = dot(evenPair, r0_yyxx * r1_wzwz * r2_zwzw * r3_xxyy) + dot(oddPair, r0_yxyx * r1_wwxy * r2_xyww * r3_zzzz) +
            dot(evenPair, r0_yxyx * r1_zzxy * r2_xyzz * r3_wwww) +
            (r0_w * dot(r1_zyx, r2_yxz * r3_xzy - r2_xzy * r3_yxz)) +
            (r0_z * dot(r1_wyx, r2_xwy * r3_yxw - r2_yxw * r3_xwy));

    return det;
}

// Returns the determinant of the matrix
template <typename T, s64 Dim, bool Packed>
T det(const mat<T, Dim, Dim, Packed> &m) {
    // Only works if L's diagonal is 1s
    s64 parity;
    auto [L, U, P] = decompose_lup(m, parity);
    T prod = U(0, 0);
    for (s64 i = 1; i < U.R; ++i) {
        prod *= U(i, i);
    }
    return parity * prod;
}

// Transposes the matrix in-place
template <typename T_, s64 R, s64 C, bool Packed>
mat<T_, C, R, Packed> T(const mat<T_, R, C, Packed> &m) {
    mat<T_, C, R, Packed> result;
    for (s64 i = 0; i < m.R; ++i) {
        for (s64 j = 0; j < m.C; ++j) result(j, i) = m(i, j);
    }
    return result;
}

// Returns the inverse of a 2x2 matrix
template <typename T, bool Packed>
auto inverse(const mat<T, 2, 2, Packed> &m) {
    mat<T, 2, 2, Packed> result;

    const auto &r0 = m.Stripes[0];
    const auto &r1 = m.Stripes[1];

    result.Stripes[0] = {r1.y, -r0.y};
    result.Stripes[1] = {-r1.x, r0.x};

    auto det = T(1) / (r0.x * r1.y - r0.y * r1.x);
    result *= det;

    return result;
}

// Returns the inverse of a 3x3 matrix
template <typename T, bool Packed>
auto inverse(const mat<T, 3, 3, Packed> &m) {
    mat<T, 3, 3, Packed> result;

    using Vec3 = vec<T, 3, false>;

    // This code below uses notation for row-major matrices' stripes.
    // It, however, "magically" works for column-major layout as well.
    Vec3 r0_zxy = m.Stripes[0].zxy;
    Vec3 r0_yzx = m.Stripes[0].yzx;
    Vec3 r1_yzx = m.Stripes[1].yzx;
    Vec3 r1_zxy = m.Stripes[1].zxy;
    Vec3 r2_zxy = m.Stripes[2].zxy;
    Vec3 r2_yzx = m.Stripes[2].yzx;

    Vec3 c0 = r1_yzx * r2_zxy - r1_zxy * r2_yzx;
    Vec3 c1 = r0_zxy * r2_yzx - r0_yzx * r2_zxy;
    Vec3 c2 = r0_yzx * r1_zxy - r0_zxy * r1_yzx;

    Vec3 r0_zyx = m.Stripes[0].zyx;
    Vec3 r1_xzy = m.Stripes[1].xzy;
    Vec3 r1_yxz = m.Stripes[1].yxz;
    Vec3 r2_yxz = m.Stripes[2].yxz;
    Vec3 r2_xzy = m.Stripes[2].xzy;

    result.Stripes[0] = {c0[0], c1[0], c2[0]};
    result.Stripes[1] = {c0[1], c1[1], c2[1]};
    result.Stripes[2] = {c0[2], c1[2], c2[2]};

    T det = T(1) / dot(r0_zyx, r1_xzy * r2_yxz - r1_yxz * r2_xzy);

    result.Stripes[0] *= det;
    result.Stripes[1] *= det;
    result.Stripes[2] *= det;

    return result;
}

// Returns the inverse of a 4x4 matrix
template <typename T, bool Packed>
auto inverse(const mat<T, 4, 4, Packed> &m) {
    mat<T, 4, 4, Packed> result;

    using Vec3 = vec<T, 3, false>;
    using Vec4 = vec<T, 4, false>;

    Vec4 even = {1, -1, 1, -1};
    Vec4 odd = {-1, 1, -1, 1};
    Vec4 evenPair = {1, -1, -1, 1};
    Vec4 oddPair = {-1, 1, 1, -1};

    const Vec4 &r0 = m.Stripes[0];
    const Vec4 &r1 = m.Stripes[1];
    const Vec4 &r2 = m.Stripes[2];
    const Vec4 &r3 = m.Stripes[3];

    Vec4 r0_wwwz = r0.wwwz;
    Vec4 r0_yxxx = r0.yxxx;
    Vec4 r0_zzyy = r0.zzyy;
    Vec4 r1_wwwz = r1.wwwz;
    Vec4 r1_yxxx = r1.yxxx;
    Vec4 r1_zzyy = r1.zzyy;
    Vec4 r2_wwwz = r2.wwwz;
    Vec4 r2_yxxx = r2.yxxx;
    Vec4 r2_zzyy = r2.zzyy;
    Vec4 r3_wwwz = r3.wwwz;
    Vec4 r3_yxxx = r3.yxxx;
    Vec4 r3_zzyy = r3.zzyy;

    Vec4 r0_wwwz_r1_yxxx = r0_wwwz * r1_yxxx;
    Vec4 r0_wwwz_r1_zzyy = r0_wwwz * r1_zzyy;
    Vec4 r0_yxxx_r1_wwwz = r0_yxxx * r1_wwwz;
    Vec4 r0_yxxx_r1_zzyy = r0_yxxx * r1_zzyy;
    Vec4 r0_zzyy_r1_wwwz = r0_zzyy * r1_wwwz;
    Vec4 r0_zzyy_r1_yxxx = r0_zzyy * r1_yxxx;
    Vec4 r2_wwwz_r3_yxxx = r2_wwwz * r3_yxxx;
    Vec4 r2_wwwz_r3_zzyy = r2_wwwz * r3_zzyy;
    Vec4 r2_yxxx_r3_wwwz = r2_yxxx * r3_wwwz;
    Vec4 r2_yxxx_r3_zzyy = r2_yxxx * r3_zzyy;
    Vec4 r2_zzyy_r3_wwwz = r2_zzyy * r3_wwwz;
    Vec4 r2_zzyy_r3_yxxx = r2_zzyy * r3_yxxx;

    Vec4 c0 = odd * (r1_wwwz * r2_zzyy_r3_yxxx - r1_zzyy * r2_wwwz_r3_yxxx - r1_wwwz * r2_yxxx_r3_zzyy +
                     r1_yxxx * r2_wwwz_r3_zzyy + r1_zzyy * r2_yxxx_r3_wwwz - r1_yxxx * r2_zzyy_r3_wwwz);
    Vec4 c1 = even * (r0_wwwz * r2_zzyy_r3_yxxx - r0_zzyy * r2_wwwz_r3_yxxx - r0_wwwz * r2_yxxx_r3_zzyy +
                      r0_yxxx * r2_wwwz_r3_zzyy + r0_zzyy * r2_yxxx_r3_wwwz - r0_yxxx * r2_zzyy_r3_wwwz);
    Vec4 c2 = odd * (r0_wwwz_r1_zzyy * r3_yxxx - r0_zzyy_r1_wwwz * r3_yxxx - r0_wwwz_r1_yxxx * r3_zzyy +
                     r0_yxxx_r1_wwwz * r3_zzyy + r0_zzyy_r1_yxxx * r3_wwwz - r0_yxxx_r1_zzyy * r3_wwwz);
    Vec4 c3 = even * (r0_wwwz_r1_zzyy * r2_yxxx - r0_zzyy_r1_wwwz * r2_yxxx - r0_wwwz_r1_yxxx * r2_zzyy +
                      r0_yxxx_r1_wwwz * r2_zzyy + r0_zzyy_r1_yxxx * r2_wwwz - r0_yxxx_r1_zzyy * r2_wwwz);

    result.Stripes[0] = {c0[0], c1[0], c2[0], c3[0]};
    result.Stripes[1] = {c0[1], c1[1], c2[1], c3[1]};
    result.Stripes[2] = {c0[2], c1[2], c2[2], c3[2]};
    result.Stripes[3] = {c0[3], c1[3], c2[3], c3[3]};

    Vec4 r2_zwzw = r2.zwzw;
    Vec4 r0_yyxx = r0.yyxx;
    Vec4 r1_wwxy = r1.wwxy;
    Vec4 r2_xyzz = r2.xyzz;
    Vec4 r3_wwww = r3.wwww;
    Vec4 r1_zzxy = r1.zzxy;
    Vec4 r0_yxyx = r0.yxyx;
    Vec4 r3_xxyy = r3.xxyy;
    Vec4 r1_wzwz = r1.wzwz;
    Vec4 r2_xyww = r2.xyww;
    Vec4 r3_zzzz = r3.zzzz;

    Vec3 r2_yxz = r2.yxz;
    Vec3 r3_xzy = r3.xzy;
    Vec3 r2_xzy = r2.xzy;
    Vec3 r3_yxz = r3.yxz;
    Vec3 r2_yxw = r2.yxw;
    Vec3 r1_zyx = r1.zyx;
    Vec3 r3_yxw = r3.yxw;
    Vec3 r2_xwy = r2.xwy;
    Vec3 r3_xwy = r3.xwy;
    Vec3 r1_wyx = r1.wyx;
    T r0_w = r0.w;
    T r0_z = r0.z;

    T det = dot(evenPair, r0_yyxx * r1_wzwz * r2_zwzw * r3_xxyy) + dot(oddPair, r0_yxyx * r1_wwxy * r2_xyww * r3_zzzz) +
            dot(evenPair, r0_yxyx * r1_zzxy * r2_xyzz * r3_wwww) +
            (r0_w * dot(r1_zyx, r2_yxz * r3_xzy - r2_xzy * r3_yxz)) +
            (r0_z * dot(r1_wyx, r2_xwy * r3_yxw - r2_yxw * r3_xwy));

    T invDet = 1 / det;

    result.Stripes[0] *= invDet;
    result.Stripes[1] *= invDet;
    result.Stripes[2] *= invDet;
    result.Stripes[3] *= invDet;

    return result;
}

// Returns the inverse of the matrix
template <typename T, s64 Dim, bool Packed>
mat<T, Dim, Dim, Packed> inverse(const mat<T, Dim, Dim, Packed> &m) {
    mat<T, Dim, Dim, Packed> result;

    auto lup = decompose_lup(m);

    vec<T, Dim, Packed> b(0);
    vec<T, Dim, Packed> x;
    for (s64 col = 0; col < Dim; ++col) {
        b[max(col - 1, 0)] = 0;
        b[col] = 1;
        x = lup.solve(b);
        for (s64 i = 0; i < Dim; ++i) {
            result(i, col) = x[i];
        }
    }

    return result;
}

// Calculates the square of the Frobenius norm of the matrix
template <typename T, s64 R, s64 C, bool Packed>
T norm_sq(const mat<T, R, C, Packed> &m) {
    T sum = T(0);
    For(range(m.StripeCount)) sum += len_sq(m.Stripes[it]);
    return sum;
}

// Calculates the Frobenius norm of the matrix
template <typename T, s64 R, s64 C, bool Packed>
T norm(const mat<T, R, C, Packed> &m) {
    return (T) Math_Sqrt_flt32(norm_sq(m));
}

// Returns the element-wise minimum of arguments
template <any_mat Mat>
always_inline Mat min(const Mat &lhs, const Mat &rhs) {
    Mat result = lhs;
    For(range(result.StripeCount)) result.Stripes[it] = min(result.Stripes[it], rhs.Stripes[it]);
    return result;
}

// Returns the element-wise maximum of arguments
template <any_mat Mat>
always_inline Mat max(const Mat &lhs, const Mat &rhs) {
    Mat result = lhs;
    For(range(result.StripeCount)) result.Stripes[it] = max(result.Stripes[it], rhs.Stripes[it]);
    return result;
}

// Clamps each vector value with specified bounds
template <any_mat Mat>
always_inline Mat clamp(const Mat &arg, typename vec_info<Mat>::T lower, typename vec_info<Mat>::T upper) {
    Mat result = arg;
    For(result.Stripes) it = clamp(it, lower, upper);
    return result;
}

// Returns the element-wise natural log of the matrix
template <any_mat Mat>
always_inline Mat ln(const Mat &mat) {
    Mat result = mat;
    For(result.Stripes) it = ln(it);
    return result;
}

// Returns the element-wise exp of the matrix
template <any_mat Mat>
always_inline Mat exp(const Mat &mat) {
    Mat result = mat;
    For(result.Stripes) it = exp(it);
    return result;
}

// Returns the element-wise sqrt of the matrix
template <any_mat Mat>
always_inline Mat sqrt(const Mat &mat) {
    Mat result = mat;
    For(result.Stripes) it = sqrt(it);
    return result;
}

// Returns the element-wise abs of the matrix
template <any_mat Mat>
always_inline Mat abs(const Mat &mat) {
    Mat result = mat;
    For(result.Stripes) it = abs(it);
    return result;
}

// Returns the sum of the elements in the matrix
template <any_mat Mat>
always_inline auto sum(const Mat &mat) {
    auto result = sum(mat.Stripes[0]);
    For(range(1, mat.Stripes.Count)) result += sum(mat.Stripes[it]);
    return result;
}

template <typename T1, typename T2, s64 R, s64 C, bool Packed1, bool Packed2>
bool almost_equal(const mat<T1, R, C, Packed1> &lhs, const mat<T2, R, C, Packed2> &rhs) {
    bool eq = true;
    For_as(i, range(R)) { For_as(j, range(C)) eq = eq && almost_equal(lhs(i, j), rhs(i, j)); }
    return eq;
}

LSTD_END_NAMESPACE
