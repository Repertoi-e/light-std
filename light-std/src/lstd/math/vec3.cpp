#include "../intrin/math.h"

#include "mat4.h"
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

LSTD_BEGIN_NAMESPACE

vec3::vec3() : x(0.0f), y(0.0f), z(0.0f) {}
vec3::vec3(f32 scalar) : x(scalar), y(scalar), z(scalar) {}
vec3::vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
vec3::vec3(const vec2 &other) : x(other.x), y(other.y), z(0.0f) {}
vec3::vec3(f32 x, f32 y) : x(x), y(y), z(0.0f) {}
vec3::vec3(const vec4 &other) : x(other.x), y(other.y), z(other.z) {}

vec3 *vec3::add(vec3 other) {
    x += other.x;
    y += other.y;
    z += other.z;

    return this;
}

vec3 *vec3::subtract(vec3 other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return this;
}

vec3 *vec3::multiply(vec3 other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;

    return this;
}

vec3 *vec3::divide(vec3 other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;

    return this;
}

vec3 *vec3::add(f32 other) {
    x += other;
    y += other;
    z += other;

    return this;
}

vec3 *vec3::subtract(f32 other) {
    x -= other;
    y -= other;
    z -= other;

    return this;
}

vec3 *vec3::multiply(f32 other) {
    x *= other;
    y *= other;
    z *= other;

    return this;
}

vec3 *vec3::divide(f32 other) {
    x /= other;
    y /= other;
    z /= other;

    return this;
}

vec3 vec3::multiply(const mat4 &transform) const {
    return vec3(transform.Rows[0].x * x + transform.Rows[0].y * y + transform.Rows[0].z * z + transform.Rows[0].w,
                transform.Rows[1].x * x + transform.Rows[1].y * y + transform.Rows[1].z * z + transform.Rows[1].w,
                transform.Rows[2].x * x + transform.Rows[2].y * y + transform.Rows[2].z * z + transform.Rows[2].w);
}

vec3 operator+(vec3 left, vec3 right) { return *left.add(right); }
vec3 operator-(vec3 left, vec3 right) { return *left.subtract(right); }
vec3 operator*(vec3 left, vec3 right) { return *left.multiply(right); }
vec3 operator/(vec3 left, vec3 right) { return *left.divide(right); }
vec3 operator+(vec3 left, f32 right) { return *left.add(right); }
vec3 operator-(vec3 left, f32 right) { return *left.subtract(right); }
vec3 operator*(vec3 left, f32 right) { return *left.multiply(right); }
vec3 operator/(vec3 left, f32 right) { return *left.divide(right); }

vec3 &vec3::operator+=(vec3 other) { return *add(other); }
vec3 &vec3::operator-=(vec3 other) { return *subtract(other); }
vec3 &vec3::operator*=(vec3 other) { return *multiply(other); }
vec3 &vec3::operator/=(vec3 other) { return *divide(other); }
vec3 &vec3::operator+=(f32 other) { return *add(other); }
vec3 &vec3::operator-=(f32 other) { return *subtract(other); }
vec3 &vec3::operator*=(f32 other) { return *multiply(other); }
vec3 &vec3::operator/=(f32 other) { return *divide(other); }

bool vec3::operator<(vec3 other) const { return x < other.x && y < other.y && z < other.z; }
bool vec3::operator<=(vec3 other) const { return x <= other.x && y <= other.y && z <= other.z; }
bool vec3::operator>(vec3 other) const { return x > other.x && y > other.y && z > other.z; }
bool vec3::operator>=(vec3 other) const { return x >= other.x && y >= other.y && z >= other.z; }
bool vec3::operator==(vec3 other) const { return x == other.x && y == other.y && z == other.z; }
bool vec3::operator!=(vec3 other) const { return !(*this == other); }

vec3 operator-(vec3 vector) { return vec3(-vector.x, -vector.y, -vector.z); }

vec3 vec3::cross(vec3 other) const {
    return vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
}

f32 vec3::dot(vec3 other) const { return x * other.x + y * other.y + z * other.z; }

f32 vec3::magnitude() const { return SQRT(x * x + y * y + z * z); }

vec3 vec3::normalize() const {
    f32 length = magnitude();
    return vec3(x / length, y / length, z / length);
}

f32 vec3::distance(vec3 other) const {
    f32 a = x - other.x;
    f32 b = y - other.y;
    f32 c = z - other.z;
    return SQRT(a * a + b * b + c * c);
}

LSTD_END_NAMESPACE