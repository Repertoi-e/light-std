#pragma once

#include "../types.h"

namespace le {

struct mat4;
struct vec3;

struct vec4 {
    union {
        struct {
            f32 x, y, z, w;
        };
        struct {
            f32 r, g, b, a;
        };
    };

    vec4() = default;
    vec4(f32 scalar);
    vec4(f32 x, f32 y, f32 z, f32 w);
    vec4(const vec3 &xyz, f32 w);

    vec4 *add(vec4 other);
    vec4 *subtract(vec4 other);
    vec4 *multiply(vec4 other);
    vec4 *divide(vec4 other);

    vec4 multiply(const mat4 &transform) const;

    friend vec4 operator+(vec4 left, vec4 right);
    friend vec4 operator-(vec4 left, vec4 right);
    friend vec4 operator*(vec4 left, vec4 right);
    friend vec4 operator/(vec4 left, vec4 right);

    bool operator==(vec4 other);
    bool operator!=(vec4 other);

    vec4 &operator+=(vec4 other);
    vec4 &operator-=(vec4 other);
    vec4 &operator*=(vec4 other);
    vec4 &operator/=(vec4 other);

    bool operator<(vec4 other) const;
    bool operator<=(vec4 other) const;
    bool operator>(vec4 other) const;
    bool operator>=(vec4 other) const;

    f32 dot(vec4 other);
};

template <typename T>
struct tvec4 {
    T x, y, z, w;

    tvec4<T>();
    tvec4<T>(T scalar);
    tvec4<T>(T x, T y, T z, T w);

    tvec4<T> *add(tvec4<T> other);
    tvec4<T> *subtract(tvec4<T> other);
    tvec4<T> *multiply(tvec4<T> other);
    tvec4<T> *divide(tvec4<T> other);

    tvec4<T> *add(T value);
    tvec4<T> *subtract(T value);
    tvec4<T> *multiply(T value);
    tvec4<T> *divide(T value);

    bool operator==(tvec4<T> other) const;
    bool operator!=(tvec4<T> other) const;

    tvec4<T> &operator+=(tvec4<T> other);
    tvec4<T> &operator-=(tvec4<T> other);
    tvec4<T> &operator*=(tvec4<T> other);
    tvec4<T> &operator/=(tvec4<T> other);

    tvec4<T> &operator+=(T value);
    tvec4<T> &operator-=(T value);
    tvec4<T> &operator*=(T value);
    tvec4<T> &operator/=(T value);

    bool operator<(tvec4<T> other) const;
    bool operator<=(tvec4<T> other) const;
    bool operator>(tvec4<T> other) const;
    bool operator>=(tvec4<T> other) const;
};

template <typename T>
tvec4<T>::tvec4() {
    this->x = 0;
    this->y = 0;
    this->z = 0;
    this->w = 0;
}

template <typename T>
tvec4<T>::tvec4(T scalar) {
    this->x = scalar;
    this->y = scalar;
    this->z = scalar;
    this->w = scalar;
}

template <typename T>
tvec4<T>::tvec4(T x, T y, T z, T w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

template <typename T>
tvec4<T> *tvec4<T>::add(tvec4<T> other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;

    return this;
}

template <typename T>
tvec4<T> *tvec4<T>::subtract(tvec4<T> other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;

    return this;
}

template <typename T>
tvec4<T> *tvec4<T>::multiply(tvec4<T> other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;

    return this;
}

template <typename T>
tvec4<T> *tvec4<T>::divide(tvec4<T> other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;

    return this;
}

template <typename T>
tvec4<T> *tvec4<T>::add(T value) {
    x += value;
    y += value;
    z += value;
    w += value;

    return this;
}

template <typename T>
tvec4<T> *tvec4<T>::subtract(T value) {
    x -= value;
    y -= value;
    z -= value;
    w -= value;

    return this;
}

template <typename T>
tvec4<T> *tvec4<T>::multiply(T value) {
    x *= value;
    y *= value;
    z *= value;
    w *= value;

    return this;
}

template <typename T>
tvec4<T> *tvec4<T>::divide(T value) {
    x /= value;
    y /= value;
    z /= value;
    w /= value;

    return this;
}

template <typename T>
bool tvec4<T>::operator==(tvec4<T> other) const {
    return x == other.x && y == other.y && z == other.z && w = other.w;
}

template <typename T>
bool tvec4<T>::operator!=(tvec4<T> other) const {
    return !(*this == other);
}

template <typename T>
tvec4<T> &tvec4<T>::operator+=(tvec4<T> other) {
    return *add(other);
}

template <typename T>
tvec4<T> &tvec4<T>::operator-=(tvec4<T> other) {
    return *subtract(other);
}

template <typename T>
tvec4<T> &tvec4<T>::operator*=(tvec4<T> other) {
    return *multiply(other);
}

template <typename T>
tvec4<T> &tvec4<T>::operator/=(tvec4<T> other) {
    return *divide(other);
}

template <typename T>
tvec4<T> &tvec4<T>::operator+=(T value) {
    return *add(value);
}

template <typename T>
tvec4<T> &tvec4<T>::operator-=(T value) {
    return *subtract(value);
}

template <typename T>
tvec4<T> &tvec4<T>::operator*=(T value) {
    return *multiply(value);
}

template <typename T>
tvec4<T> &tvec4<T>::operator/=(T value) {
    return *divide(value);
}

template <typename T>
bool tvec4<T>::operator<(tvec4<T> other) const {
    return x < other.x && y < other.y && z < other.z && w < other.w;
}

template <typename T>
bool tvec4<T>::operator<=(tvec4<T> other) const {
    return x <= other.x && y <= other.y && z <= other.z && w <= other.w;
}

template <typename T>
bool tvec4<T>::operator>(tvec4<T> other) const {
    return !(*this < other);
}

template <typename T>
bool tvec4<T>::operator>=(tvec4<T> other) const {
    return !(*this <= other);
}

template <typename T>
tvec4<T> operator+(tvec4<T> left, tvec4<T> right) {
    return *left.add(right);
}

template <typename T>
tvec4<T> operator-(tvec4<T> left, tvec4<T> right) {
    return *left.subtract(right);
}

template <typename T>
tvec4<T> operator*(tvec4<T> left, tvec4<T> right) {
    return *left.multiply(right);
}

template <typename T>
tvec4<T> operator/(tvec4<T> left, tvec4<T> right) {
    return *left.divide(right);
}

template <typename T>
tvec4<T> operator+(tvec4<T> left, T value) {
    return *left.add(value);
}

template <typename T>
tvec4<T> operator-(tvec4<T> left, T value) {
    return *left.subtract(value);
}

template <typename T>
tvec4<T> operator*(tvec4<T> left, T value) {
    return *left.multiply(value);
}

template <typename T>
tvec4<T> operator/(tvec4<T> left, T value) {
    return *left.divide(value);
}
}  // namespace le
