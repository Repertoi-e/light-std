#pragma once

#include "../types.h"

namespace le {

struct vec2;
struct vec4;
struct mat4;

struct vec3 {
    f32 x, y, z;

    vec3();
    vec3(f32 scalar);
    vec3(f32 x, f32 y, f32 z);
    vec3(const vec2 &other);
    vec3(f32 x, f32 y);
    vec3(const vec4 &other);

    static vec3 UP();
    static vec3 DOWN();
    static vec3 LEFT();
    static vec3 RIGHT();
    static vec3 ZERO();

    static vec3 X();
    static vec3 Y();
    static vec3 Z();

    vec3 *add(vec3 other);
    vec3 *subtract(vec3 other);
    vec3 *multiply(vec3 other);
    vec3 *divide(vec3 other);

    vec3 *add(f32 other);
    vec3 *subtract(f32 other);
    vec3 *multiply(f32 other);
    vec3 *divide(f32 other);

    vec3 multiply(const mat4 &transform) const;

    friend vec3 operator+(vec3 left, vec3 right);
    friend vec3 operator-(vec3 left, vec3 right);
    friend vec3 operator*(vec3 left, vec3 right);
    friend vec3 operator/(vec3 left, vec3 right);

    friend vec3 operator+(vec3 left, f32 right);
    friend vec3 operator-(vec3 left, f32 right);
    friend vec3 operator*(vec3 left, f32 right);
    friend vec3 operator/(vec3 left, f32 right);

    bool operator==(vec3 other) const;
    bool operator!=(vec3 other) const;

    vec3 &operator+=(vec3 other);
    vec3 &operator-=(vec3 other);
    vec3 &operator*=(vec3 other);
    vec3 &operator/=(vec3 other);

    vec3 &operator+=(f32 other);
    vec3 &operator-=(f32 other);
    vec3 &operator*=(f32 other);
    vec3 &operator/=(f32 other);

    bool operator<(vec3 other) const;
    bool operator<=(vec3 other) const;
    bool operator>(vec3 other) const;
    bool operator>=(vec3 other) const;

    vec3 cross(vec3 other) const;
    f32 dot(vec3 other) const;

    f32 magnitude() const;
    vec3 normalize() const;
    f32 distance(vec3 other) const;
};

template <typename T>
struct tvec3 {
    T x, y, z;

    tvec3<T>();
    tvec3<T>(T scalar);
    tvec3<T>(T x, T y, T z);

    tvec3<T> *add(tvec3<T> other);
    tvec3<T> *subtract(tvec3<T> other);
    tvec3<T> *multiply(tvec3<T> other);
    tvec3<T> *divide(tvec3<T> other);

    tvec3<T> *add(T value);
    tvec3<T> *subtract(T value);
    tvec3<T> *multiply(T value);
    tvec3<T> *divide(T value);

    bool operator==(tvec3<T> other) const;
    bool operator!=(tvec3<T> other) const;

    tvec3<T> &operator+=(tvec3<T> other);
    tvec3<T> &operator-=(tvec3<T> other);
    tvec3<T> &operator*=(tvec3<T> other);
    tvec3<T> &operator/=(tvec3<T> other);

    tvec3<T> &operator+=(T value);
    tvec3<T> &operator-=(T value);
    tvec3<T> &operator*=(T value);
    tvec3<T> &operator/=(T value);

    bool operator<(tvec3<T> other) const;
    bool operator<=(tvec3<T> other) const;
    bool operator>(tvec3<T> other) const;
    bool operator>=(tvec3<T> other) const;
};

template <typename T>
tvec3<T>::tvec3() {
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

template <typename T>
tvec3<T>::tvec3(T scalar) {
    this->x = scalar;
    this->y = scalar;
    this->z = scalar;
}

template <typename T>
tvec3<T>::tvec3(T x, T y, T z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

template <typename T>
tvec3<T> *tvec3<T>::add(tvec3<T> other) {
    x += other.x;
    y += other.y;
    z += other.z;

    return this;
}

template <typename T>
tvec3<T> *tvec3<T>::subtract(tvec3<T> other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return this;
}

template <typename T>
tvec3<T> *tvec3<T>::multiply(tvec3<T> other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;

    return this;
}

template <typename T>
tvec3<T> *tvec3<T>::divide(tvec3<T> other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;

    return this;
}

template <typename T>
tvec3<T> *tvec3<T>::add(T value) {
    x += value;
    y += value;
    z += value;

    return this;
}

template <typename T>
tvec3<T> *tvec3<T>::subtract(T value) {
    x -= value;
    y -= value;
    z -= value;

    return this;
}

template <typename T>
tvec3<T> *tvec3<T>::multiply(T value) {
    x *= value;
    y *= value;
    z *= value;

    return this;
}

template <typename T>
tvec3<T> *tvec3<T>::divide(T value) {
    x /= value;
    y /= value;
    z /= value;

    return this;
}

template <typename T>
bool tvec3<T>::operator==(tvec3<T> other) const {
    return x == other.x && y == other.y && z == other.z;
}

template <typename T>
bool tvec3<T>::operator!=(tvec3<T> other) const {
    return !(*this == other);
}

template <typename T>
tvec3<T> &tvec3<T>::operator+=(tvec3<T> other) {
    return *add(other);
}

template <typename T>
tvec3<T> &tvec3<T>::operator-=(tvec3<T> other) {
    return *subtract(other);
}

template <typename T>
tvec3<T> &tvec3<T>::operator*=(tvec3<T> other) {
    return *multiply(other);
}

template <typename T>
tvec3<T> &tvec3<T>::operator/=(tvec3<T> other) {
    return *divide(other);
}

template <typename T>
tvec3<T> &tvec3<T>::operator+=(T value) {
    return *add(value);
}

template <typename T>
tvec3<T> &tvec3<T>::operator-=(T value) {
    return *subtract(value);
}

template <typename T>
tvec3<T> &tvec3<T>::operator*=(T value) {
    return *multiply(value);
}

template <typename T>
tvec3<T> &tvec3<T>::operator/=(T value) {
    return *divide(value);
}

template <typename T>
bool tvec3<T>::operator<(tvec3<T> other) const {
    return x < other.x && y < other.y && z < other.z;
}

template <typename T>
bool tvec3<T>::operator<=(tvec3<T> other) const {
    return x <= other.x && y <= other.y && z <= other.z;
}

template <typename T>
bool tvec3<T>::operator>(tvec3<T> other) const {
    return !(*this < other);
}

template <typename T>
bool tvec3<T>::operator>=(tvec3<T> other) const {
    return !(*this <= other);
}

template <typename T>
tvec3<T> operator+(tvec3<T> left, tvec3<T> right) {
    return *left.add(right);
}

template <typename T>
tvec3<T> operator-(tvec3<T> left, tvec3<T> right) {
    return *left.subtract(right);
}

template <typename T>
tvec3<T> operator*(tvec3<T> left, tvec3<T> right) {
    return *left.multiply(right);
}

template <typename T>
tvec3<T> operator/(tvec3<T> left, tvec3<T> right) {
    return *left.divide(right);
}

template <typename T>
tvec3<T> operator+(tvec3<T> left, T value) {
    return *left.add(value);
}

template <typename T>
tvec3<T> operator-(tvec3<T> left, T value) {
    return *left.subtract(value);
}

template <typename T>
tvec3<T> operator*(tvec3<T> left, T value) {
    return *left.multiply(value);
}

template <typename T>
tvec3<T> operator/(tvec3<T> left, T value) {
    return *left.divide(value);
}
}  // namespace le