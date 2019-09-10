#pragma once

#include "../types.h"

#define PI 3.1415926535897932384626433832795f
#define PI_OVER_2 1.57079632679489661923f   // pi/2
#define PI_OVER_4 0.785398163397448309616f  // pi/4

#define LN_BASE 2.71828182845904523536f  // e

#define TAU 6.283185307179586476925286766559f

#define LOG2 0.69314718055994530941723212145818f
#define LOG10 2.30258509299404568402f

#define LOG2E 1.44269504088896340736f    // log2(e)
#define LOG10E 0.434294481903251827651f  // log10(e)

#define SQRT2 1.41421356237309504880f       // sqrt(2)
#define INV_SQRT2 0.707106781186547524401f  // 1/sqrt(2)

// @TODO Replace with our own math functions in order to not depend on the runtime lib
#include <math.h>

template <typename T>
enable_if_t<is_floating_point_v<T>, T> TO_RAD(T degrees) {
    return (T)(degrees * (PI / 180.0f));
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> TO_DEG(T radians) {
    return (T)(radians * (180.0f / PI));
}

template <typename T>
s32 SIGN(T value) {
    return (value > 0) - (value < 0);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> SIN(T angle) {
    return (T)::sin(angle);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> COS(T angle) {
    return (T)::cos(angle);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> TAN(T angle) {
    return (T)::tan(angle);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> SQRT(T value) {
    return (T)::sqrt(value);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> INV_SQRT(T value) {
    return T(1) / (T)::sqrt(value);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> ASIN(T value) {
    return (T)::asin(value);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> ACOS(T value) {
    return (T)::acos(value);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> ATAN(T value) {
    return (T)::atan(value);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> ATAN2(T y, T x) {
    return (T)::atan2(y, x);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> LOG_2(T x) {
    return (T)::log2(x);
}

template <typename T>
enable_if_t<is_floating_point_v<T>, T> CLAMP(T value, T minimum, T maximum) {
    return (value > minimum) ? (value < maximum) ? value : maximum : minimum;
}
