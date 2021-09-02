#pragma once

#include "../memory/stack_array.h"
#include "vec.h"

LSTD_BEGIN_NAMESPACE

template <typename MatrixT>
struct mat_info_helper {
    static constexpr bool IS_MAT = false;
};

template <typename T_, s64 R_, s64 C_, bool Packed_>
struct mat_info_helper<mat<T_, R_, C_, Packed_>> {
    static constexpr bool IS_MAT = true;

    using T = T_;

    static constexpr s64 R       = R_;
    static constexpr s64 C       = C_;
    static constexpr bool PACKED = Packed_;
};

template <typename MatrixT>
struct mat_info : public mat_info_helper<types::decay_t<MatrixT>> {
};

// @TODO: Use this inplace of templates.
// I guess we did this for vectors but not fully for matrices.
template <typename T>
concept any_mat = mat_info<T>::IS_MAT;

template <typename T, typename U>
using mat_mul_elem_t = decltype(T() * U() + T() * U());

template <typename T, s64 R_, s64 C_, bool Packed = false>
struct mat_data {
    static constexpr s64 C      = C_;
    static constexpr s64 R      = R_;
    static constexpr s64 Width  = C_;
    static constexpr s64 Height = R_;

    static constexpr s64 StripeDim   = C;
    static constexpr s64 StripeCount = R;

    using StripeVecT = vec<T, StripeDim, Packed>;
    stack_array<StripeVecT, StripeCount> Stripes;

protected:
    always_inline T &get_element(s64 row, s64 col) {
        s64 r = translate_index(row, R);
        s64 c = translate_index(col, C);
        return Stripes[r][c];
    }

    always_inline T get_element(s64 row, s64 col) const {
        s64 r = translate_index(row, R);
        s64 c = translate_index(col, C);
        return Stripes[r][c];
    }
};

template <typename MatrixT, s64 SR, s64 SC>
struct mat_view : non_copyable {
    using Info = mat_info<MatrixT>;

    static constexpr s64 VecDim         = max(SR, SC);
    static constexpr bool VecAssignable = min(SR, SC) == 1;

    MatrixT &Mat;
    s64 Row = -1, Col = -1;

    mat_view(MatrixT &mat, s64 row, s64 col)
        : Mat(mat),
          Row(row),
          Col(col) {
    }

    mat_view(mat_view &&rhs)
        : Mat(rhs.Mat),
          Row(rhs.Row),
          Col(rhs.Col) {
    }

    template <typename U, bool UPacked>
    operator mat<U, SR, SC, UPacked>() const {
        mat<U, SR, SC, UPacked> result;
        For_as(i, range(SR)) { For_as(j, range(SC)) result(i, j) = (U) (*this)(i, j); }
        return result;
    }

    template <typename U, bool Packed2>
        requires(VecAssignable)
    operator vec<U, VecDim, Packed2>() const {
        vec<U, max(SR, SC), Packed2> v;
        s64 k = 0;
        For_as(i, range(SR)) {
            For_as(j, range(SC)) {
                v[k] = (U) (*this)(i, j);
                ++k;
            }
        }
        return v;
    }

    template <typename U, bool UPacked>
    mat_view &operator=(const mat<U, SR, SC, UPacked> &rhs) {
        static_assert(!types::is_const<MatrixT>, "Cannot assign to submatrix of const matrix.");

        // If aliasing happens, the same matrix is copied to itself with no side-effects
        For_as(i, range(SR)) {
            For_as(j, range(SC)) Mat(Row + i, Col + j) = (typename mat_info<MatrixT>::T) rhs(i, j);
        }
        return *this;
    }

    // From vector if applicable (for 1*N and N*1 submatrices)
    template <typename U, bool Packed>
        requires(VecAssignable)
    mat_view &operator=(const vec<U, VecDim, Packed> &v) {
        static_assert(!types::is_const<MatrixT>, "Cannot assign to submatrix of const matrix.");

        s64 k = 0;
        For_as(i, range(SR)) {
            For_as(j, range(SC)) {
                Mat(Row + i, Col + j) = (typename mat_info<MatrixT>::T) v[k];
                ++k;
            }
        }
        return *this;
    }

    template <typename MatrixU>
    mat_view &operator=(const mat_view<MatrixU, SR, SC> &rhs) {
        static_assert(!types::is_const<MatrixT>, "Cannot assign to submatrix of const matrix.");

        // If *this and rhs reference the same matrix, aliasing must be resolved.
        if ((void *) &Mat == (void *) &rhs.Mat) {
            mat<typename mat_info<MatrixU>::T, SR, SC, mat_info<MatrixU>::PACKED> temp = rhs;
            operator=(temp);
        } else {
            For_as(i, range(SR)) {
                For_as(j, range(SC)) { Mat(Row + i, Col + j) = rhs(i, j); }
            }
        }
        return *this;
    }

    mat_view &operator=(const mat_view &rhs) {
        static_assert(!types::is_const<MatrixT>, "Cannot assign to submatrix of const matrix.");
        return operator=<MatrixT>(rhs);
    }

    typename Info::T &operator()(s64 row, s64 col) { return Mat(this->Row + row, this->Col + col); }
    typename Info::T operator()(s64 row, s64 col) const { return Mat(this->Row + row, this->Col + col); }
};

#if THE_COMPILER == MSVC
#define OPTIMIZATION __declspec(empty_bases)
#else
#define OPTIMIZATION
#endif

template <typename T, s64 R_, s64 C_, bool Packed = false>
struct OPTIMIZATION mat : public mat_data<T, R_, C_, Packed> {
#undef OPTIMIZATION
    static_assert(C_ >= 1 && R_ >= 1, "Dimensions must be positive integers.");

    using typename mat_data<T, R_, C_, Packed>::StripeVecT;

    static constexpr s64 VecDim         = max(R_, C_);
    static constexpr bool VecAssignable = min(R_, C_) == 1;

    using mat_data<T, R_, C_, Packed>::R;
    using mat_data<T, R_, C_, Packed>::C;

    using mat_data<T, R_, C_, Packed>::Width;
    using mat_data<T, R_, C_, Packed>::Height;

    using mat_data<T, R_, C_, Packed>::Stripes;
    using mat_data<T, R_, C_, Packed>::StripeCount;

    using mat_data<T, R_, C_, Packed>::get_element;

    struct FromStripes_ {
    };

    static constexpr FromStripes_ FromStripes = {};

    // :MathTypesNoInit By default we don't init (to save on performance) but you can call a constructor with a scalar value of 0 to zero-init.
    mat() {
    }

    template <types::is_scalar H>
    mat(H h) {
        For(Stripes) it = StripeVecT(h);
    }

    template <typename T2, bool Packed2>
    mat(const mat<T2, R, C, Packed2> &rhs) {
        For_as(i, range(R)) { For_as(j, range(C)) (*this)(i, j) = rhs(i, j); }
    }

    // Requires all of the arguments to be scalars and their count be R * C.
    template <types::is_scalar H, types::is_scalar... Args>
        requires(1 + sizeof...(Args) == R * C)
    mat(H h, Args ... args) {
        assign<0, 0>(h, args...);
    }

    // From vector if applicable (for 1*N and N*1 matrices)
    template <typename T2, bool Packed2>
        requires(VecAssignable)
    mat(const vec<T2, VecDim, Packed2> &v) {
        For(range(v.DIM)) (*this)(it) = v[it];
    }

    // Used by internal methods
    template <typename... Stripes>
    mat(FromStripes_, Stripes ... stripes)
        : mat_data<T, R, C, Packed>{(Stripes &&) stripes...} {
    }

    // General matrix indexing
    always_inline T &operator()(s64 row, s64 col) { return get_element(row, col); }
    always_inline T operator()(s64 row, s64 col) const { return get_element(row, col); }

    // Column and row vector simple indexing
    template <typename Q = T>
        requires(C == 1 && R > 1 || C > 1 && R == 1)
    always_inline T &operator()(s64 index) {
        return get_element(R == 1 ? 0 : index, C == 1 ? 0 : index);
    }

    template <typename Q = T>
        requires(C == 1 && R > 1 || C > 1 && R == 1)
    always_inline T &operator()(s64 index) const {
        return get_element(R == 1 ? 0 : index, C == 1 ? 0 : index);
    }

    template <s64 SR, s64 SC>
    mat_view<mat, SR, SC> get_view(s64 row, s64 col) {
        s64 r = translate_index(row, R);
        s64 c = translate_index(col, C);
        assert(SR + r <= R);
        assert(SC + c <= C);
        return mat_view<mat, SR, SC>(*this, r, c);
    }

    template <s64 SR, s64 SC>
    mat_view<const mat, SR, SC> get_view(s64 row, s64 col) const {
        s64 r = translate_index(row, R);
        s64 c = translate_index(col, C);
        assert(SR + r <= R);
        assert(SC + c <= C);
        return mat_view<const mat, SR, SC>(*this, r, c);
    }

    template <s64 SC>
    mat_view<mat, R, SC> get_column_view(s64 col) {
        s64 c = translate_index(col, C);
        assert(SC + c <= C);
        return mat_view<mat, R, SC>(*this, 0, c);
    }

    template <s64 SC>
    mat_view<const mat, R, SC> get_column_view(s64 col) const {
        s64 c = translate_index(col, C);
        assert(SC + c <= C);
        return mat_view<const mat, R, SC>(*this, 0, c);
    }

    template <s64 SR>
    mat_view<mat, SR, C> get_row_view(s64 row) {
        s64 r = translate_index(row, R);
        assert(SR + r <= R);
        return mat_view<mat, SR, C>(*this, r, 0);
    }

    template <s64 SR>
    mat_view<const mat, SR, C> get_row_view(s64 row) const {
        s64 r = translate_index(row, R);
        assert(SR + r <= R);
        return mat_view<const mat, SR, C>(*this, r, 0);
    }

    auto col(s64 col) { return get_view<R, 1>(0, col); }
    auto row(s64 row) { return get_view<1, C>(row, 0); }
    auto col(s64 col) const { return get_view<R, 1>(0, col); }
    auto row(s64 row) const { return get_view<1, C>(row, 0); }

    // Conversion to vector if applicable
    template <typename T2, bool Packed2>
        requires(VecAssignable)
    operator vec<T2, VecDim, Packed2>() const {
        vec<T2, max(R, C), Packed2> v;
        s64 k = 0;
        For_as(i, range(R)) {
            For_as(j, range(C)) {
                v[k] = (*this)(i, j);
                ++k;
            }
        }
        return v;
    }

private:
    template <s64 i, s64 j, typename Head, typename... Args>
    void assign(Head head, Args ... args) {
        (*this)(i, j) = (T) head;
        assign<j != C - 1 ? i : i + 1, (j + 1) % C>(args...);
    }

    template <s64, s64>
    void assign() {
    }
};

template <s64 R, s64 C, bool Packed = false>
using matf = mat<f32, R, C, Packed>;

using m22 = matf<2, 2>;
using m23 = matf<2, 3>;
using m33 = matf<3, 3>;
using m32 = matf<3, 2>;
using m44 = matf<4, 4>;

LSTD_END_NAMESPACE
