#include "vec2.h"

#include "../intrin/math.h"

LSTD_BEGIN_NAMESPACE

vec2::vec2() {
    this->x = 0;
    this->y = 0;
}

vec2::vec2(f32 scalar) {
    this->x = scalar;
    this->y = scalar;
}

vec2::vec2(f32 x, f32 y) {
    this->x = x;
    this->y = y;
}

vec2 *vec2::add(vec2 other) {
    x += other.x;
    y += other.y;

    return this;
}

vec2 *vec2::subtract(vec2 other) {
    x -= other.x;
    y -= other.y;

    return this;
}

vec2 *vec2::multiply(vec2 other) {
    x *= other.x;
    y *= other.y;

    return this;
}

vec2 *vec2::divide(vec2 other) {
    x /= other.x;
    y /= other.y;

    return this;
}

vec2 *vec2::add(f32 value) {
    x += value;
    y += value;

    return this;
}

vec2 *vec2::subtract(f32 value) {
    x -= value;
    y -= value;

    return this;
}

vec2 *vec2::multiply(f32 value) {
    x *= value;
    y *= value;

    return this;
}

vec2 *vec2::divide(f32 value) {
    x /= value;
    y /= value;

    return this;
}

vec2 operator+(vec2 left, vec2 right) { return *left.add(right); }
vec2 operator-(vec2 left, vec2 right) { return *left.subtract(right); }
vec2 operator*(vec2 left, vec2 right) { return *left.multiply(right); }
vec2 operator/(vec2 left, vec2 right) { return *left.divide(right); }

vec2 operator+(vec2 left, f32 value) { return vec2(left.x + value, left.y + value); }
vec2 operator-(vec2 left, f32 value) { return vec2(left.x - value, left.y - value); }
vec2 operator*(vec2 left, f32 value) { return vec2(left.x * value, left.y * value); }
vec2 operator/(vec2 left, f32 value) { return vec2(left.x / value, left.y / value); }

vec2 &vec2::operator+=(vec2 other) { return *add(other); }
vec2 &vec2::operator-=(vec2 other) { return *subtract(other); }
vec2 &vec2::operator*=(vec2 other) { return *multiply(other); }
vec2 &vec2::operator/=(vec2 other) { return *divide(other); }

vec2 &vec2::operator+=(f32 value) { return *add(value); }
vec2 &vec2::operator-=(f32 value) { return *subtract(value); }
vec2 &vec2::operator*=(f32 value) { return *multiply(value); }
vec2 &vec2::operator/=(f32 value) { return *divide(value); }

bool vec2::operator==(vec2 other) const { return x == other.x && y == other.y; }
bool vec2::operator!=(vec2 other) const { return !(*this == other); }
bool vec2::operator<(vec2 other) const { return x < other.x && y < other.y; }
bool vec2::operator<=(vec2 other) const { return x <= other.x && y <= other.y; }
bool vec2::operator>(vec2 other) const { return x > other.x && y > other.y; }
bool vec2::operator>=(vec2 other) const { return x >= other.x && y >= other.y; }

f32 vec2::distance(vec2 other) const {
    f32 a = x - other.x;
    f32 b = y - other.y;
    return SQRT(a * a + b * b);
}

f32 vec2::dot(vec2 other) const { return x * other.x + y * other.y; }

f32 vec2::magnitude() const { return SQRT(x * x + y * y); }

vec2 vec2::normalize() const {
    f32 length = magnitude();
    return vec2(x / length, y / length);
}

LSTD_END_NAMESPACE
