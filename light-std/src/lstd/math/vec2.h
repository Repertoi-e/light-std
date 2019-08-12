#pragma once

#include "../types.h"

namespace le {

struct vec2 {
    f32 x, y;

    vec2();
    vec2(f32 scalar);
    vec2(f32 x, f32 y);

    vec2 *add(vec2 other);
    vec2 *subtract(vec2 other);
    vec2 *multiply(vec2 other);
    vec2 *divide(vec2 other);

    vec2 *add(f32 value);
    vec2 *subtract(f32 value);
    vec2 *multiply(f32 value);
    vec2 *divide(f32 value);

    friend vec2 operator+(vec2 left, vec2 right);
    friend vec2 operator-(vec2 left, vec2 right);
    friend vec2 operator*(vec2 left, vec2 right);
    friend vec2 operator/(vec2 left, vec2 right);

    friend vec2 operator+(vec2 left, f32 value);
    friend vec2 operator-(vec2 left, f32 value);
    friend vec2 operator*(vec2 left, f32 value);
    friend vec2 operator/(vec2 left, f32 value);

    bool operator==(vec2 other) const;
    bool operator!=(vec2 other) const;

    vec2 &operator+=(vec2 other);
    vec2 &operator-=(vec2 other);
    vec2 &operator*=(vec2 other);
    vec2 &operator/=(vec2 other);

    vec2 &operator+=(f32 value);
    vec2 &operator-=(f32 value);
    vec2 &operator*=(f32 value);
    vec2 &operator/=(f32 value);

    bool operator<(vec2 other) const;
    bool operator<=(vec2 other) const;
    bool operator>(vec2 other) const;
    bool operator>=(vec2 other) const;

    f32 magnitude() const;
    vec2 normalize() const;
    f32 distance(vec2 other) const;
    f32 dot(vec2 other) const;
};

template <typename T>
struct tvec2 {
    T x, y;

    tvec2<T>();
    tvec2<T>(T scalar);
    tvec2<T>(T x, T y);

    tvec2<T> &add(tvec2<T> other);
    tvec2<T> &subtract(tvec2<T> other);
    tvec2<T> &multiply(tvec2<T> other);
    tvec2<T> &divide(tvec2<T> other);

    tvec2<T> &add(T value);
    tvec2<T> &subtract(T value);
    tvec2<T> &multiply(T value);
    tvec2<T> &divide(T value);

    bool operator==(tvec2<T> other) const;
    bool operator!=(tvec2<T> other) const;

    tvec2<T> &operator+=(tvec2<T> other);
    tvec2<T> &operator-=(tvec2<T> other);
    tvec2<T> &operator*=(tvec2<T> other);
    tvec2<T> &operator/=(tvec2<T> other);

    tvec2<T> &operator+=(T value);
    tvec2<T> &operator-=(T value);
    tvec2<T> &operator*=(T value);
    tvec2<T> &operator/=(T value);

    bool operator<(tvec2<T> other) const;
    bool operator<=(tvec2<T> other) const;
    bool operator>(tvec2<T> other) const;
    bool operator>=(tvec2<T> other) const;
};

template <typename T>
tvec2<T>::tvec2() {
    this->x = 0;
    this->y = 0;
}

template <typename T>
tvec2<T>::tvec2(T scalar) {
    this->x = scalar;
    this->y = scalar;
}

template <typename T>
tvec2<T>::tvec2(T x, T y) {
    this->x = x;
    this->y = y;
}

template <typename T>
tvec2<T> &tvec2<T>::add(tvec2<T> other) {
    x += other.x;
    y += other.y;

    return this;
}

template <typename T>
tvec2<T> &tvec2<T>::subtract(tvec2<T> other) {
    x -= other.x;
    y -= other.y;

    return this;
}

template <typename T>
tvec2<T> &tvec2<T>::multiply(tvec2<T> other) {
    x *= other.x;
    y *= other.y;

    return this;
}

template <typename T>
tvec2<T> &tvec2<T>::divide(tvec2<T> other) {
    x /= other.x;
    y /= other.y;

    return this;
}

template <typename T>
tvec2<T> &tvec2<T>::add(T value) {
    x += value;
    y += value;

    return this;
}

template <typename T>
tvec2<T> &tvec2<T>::subtract(T value) {
    x -= value;
    y -= value;

    return this;
}

template <typename T>
tvec2<T> &tvec2<T>::multiply(T value) {
    x *= value;
    y *= value;

    return this;
}

template <typename T>
tvec2<T> &tvec2<T>::divide(T value) {
    x /= value;
    y /= value;

    return this;
}

template <typename T>
bool tvec2<T>::operator==(tvec2<T> other) const {
    return x == other.x && y == other.y;
}

template <typename T>
bool tvec2<T>::operator!=(tvec2<T> other) const {
    return !(*this == other);
}

template <typename T>
tvec2<T> &tvec2<T>::operator+=(tvec2<T> other) {
    return add(other);
}

template <typename T>
tvec2<T> &tvec2<T>::operator-=(tvec2<T> other) {
    return subtract(other);
}

template <typename T>
tvec2<T> &tvec2<T>::operator*=(tvec2<T> other) {
    return multiply(other);
}

template <typename T>
tvec2<T> &tvec2<T>::operator/=(tvec2<T> other) {
    return divide(other);
}

template <typename T>
tvec2<T> &tvec2<T>::operator+=(T value) {
    return add(value);
}

template <typename T>
tvec2<T> &tvec2<T>::operator-=(T value) {
    return subtract(value);
}

template <typename T>
tvec2<T> &tvec2<T>::operator*=(T value) {
    return multiply(value);
}

template <typename T>
tvec2<T> &tvec2<T>::operator/=(T value) {
    return divide(value);
}

template <typename T>
bool tvec2<T>::operator<(tvec2<T> other) const {
    return x < other.x && y < other.y;
}

template <typename T>
bool tvec2<T>::operator<=(tvec2<T> other) const {
    return x <= other.x && y <= other.y;
}

template <typename T>
bool tvec2<T>::operator>(tvec2<T> other) const {
    return !(*this < other);
}

template <typename T>
bool tvec2<T>::operator>=(tvec2<T> other) const {
    return !(*this <= other);
}

template <typename T>
tvec2<T> operator+(tvec2<T> left, tvec2<T> right) {
    return left.add(right);
}

template <typename T>
tvec2<T> operator-(tvec2<T> left, tvec2<T> right) {
    return left.subtract(right);
}

template <typename T>
tvec2<T> operator*(tvec2<T> left, tvec2<T> right) {
    return left.multiply(right);
}

template <typename T>
tvec2<T> operator/(tvec2<T> left, tvec2<T> right) {
    return left.divide(right);
}

template <typename T>
tvec2<T> operator+(tvec2<T> left, T value) {
    return left.add(value);
}

template <typename T>
tvec2<T> operator-(tvec2<T> left, T value) {
    return left.subtract(value);
}

template <typename T>
tvec2<T> operator*(tvec2<T> left, T value) {
    return left.multiply(value);
}

template <typename T>
tvec2<T> operator/(tvec2<T> left, T value) {
    return left.divide(value);
}
}  // namespace le