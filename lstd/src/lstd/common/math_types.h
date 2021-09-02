#pragma once

#include "scalar_types.h"

LSTD_BEGIN_NAMESPACE

//
// This file provides forward definitions for math types: vec_data, vec, swizzle, mat, mat_view, tquat,
//  as well as concepts:
//  - is_vec, is_swizzle, is_vec_or_swizzle,
//  - is_mat, is_mat_view
//  - is_quat
//

template <typename T, s64 Dim, bool Packed>
struct vec_data;

template <typename T, s64 Dim, bool Packed>
struct vec;

template <typename T, s64... Indices>
struct swizzle;

template <typename T, s64 Rows, s64 Columns, bool Packed>
struct mat;

template <typename MatrixT, s64 SRows, s64 SColumns>
struct mat_view;

template <typename T, bool Packed>
struct tquat;

namespace types {
template <typename T>
struct is_vec_helper {
    static constexpr bool value = false;
};

template <typename T, s64 Dim, bool Packed>
struct is_vec_helper<vec<T, Dim, Packed>> {
    static constexpr bool value = true;
};

template <typename T>
concept is_vec = is_vec_helper<T>::value;

template <typename T>
struct is_swizzle_helper {
    static constexpr bool value = false;
};

template <typename T, s64... Indices>
struct is_swizzle_helper<swizzle<T, Indices...>> {
    static constexpr bool value = true;
};

template <typename T>
concept is_swizzle = is_swizzle_helper<T>::value;

template <typename T>
concept is_vec_or_swizzle = is_vec<T> || is_swizzle<T>;

template <typename T>
struct is_mat_helper {
    static constexpr bool value = false;
};

template <typename T, s64 R, s64 C, bool Packed>
struct is_mat_helper<mat<T, R, C, Packed>> {
    static constexpr bool value = true;
};

template <typename T>
concept is_mat = is_mat_helper<T>::value;

template <typename T>
struct is_mat_view_helper {
    static constexpr bool value = false;
};

template <typename M, s64 R, s64 C>
struct is_mat_view_helper<mat_view<M, R, C>> {
    static constexpr bool value = true;
};

template <typename T>
concept is_mat_view = is_mat_view_helper<T>::value;

template <typename T>
struct is_quat_helper {
    static constexpr bool value = false;
};

template <typename T, bool Packed>
struct is_quat_helper<tquat<T, Packed>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_quat = is_quat_helper<T>::value;

} // namespace types

LSTD_END_NAMESPACE
