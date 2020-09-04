#pragma once

#include "../internal/namespace.h"
#include "../platform.h"

LSTD_BEGIN_NAMESPACE

//
// Math types:
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
struct is_vec {
    static constexpr bool value = false;
};

template <typename T, s64 Dim, bool Packed>
struct is_vec<vec<T, Dim, Packed>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_vec_v = is_vec<T>::value;

template <typename T>
struct is_swizzle {
    static constexpr bool value = false;
};
template <typename T, s64... Indices>
struct is_swizzle<swizzle<T, Indices...>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_swizzle_v = is_swizzle<T>::value;

template <typename T>
struct is_mat {
    static constexpr bool value = false;
};

template <typename T, s64 R, s64 C, bool Packed>
struct is_mat<mat<T, R, C, Packed>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_mat_v = is_mat<T>::value;

template <typename T>
struct is_mat_view {
    static constexpr bool value = false;
};
template <typename M, s64 R, s64 C>
struct is_mat_view<mat_view<M, R, C>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_mat_view_v = is_mat_view<T>::value;

template <typename T>
struct is_quat {
    static constexpr bool value = false;
};

template <typename T, bool Packed>
struct is_quat<tquat<T, Packed>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_quat_v = is_quat<T>::value;

template <typename T>
constexpr bool is_vec_or_swizzle_v = is_vec_v<T> || is_swizzle_v<T>;
}  // namespace types
LSTD_END_NAMESPACE