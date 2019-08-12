#include "vec4.h"

#include "../intrin/math.h"

#include "mat4.h"
#include "vec3.h"

namespace le {

vec4::vec4(f32 scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
vec4::vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
vec4::vec4(const vec3 &xyz, f32 w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

vec4 *vec4::add(vec4 other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;

    return this;
}

vec4 *vec4::subtract(vec4 other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;

    return this;
}

vec4 *vec4::multiply(vec4 other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;

    return this;
}

vec4 *vec4::divide(vec4 other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;

    return this;
}

vec4 vec4::multiply(const mat4 &transform) const {
    return vec4(transform.Rows[0].x * x + transform.Rows[0].y * y + transform.Rows[0].z * z + transform.Rows[0].w * w,
                transform.Rows[1].x * x + transform.Rows[1].y * y + transform.Rows[1].z * z + transform.Rows[1].w * w,
                transform.Rows[2].x * x + transform.Rows[2].y * y + transform.Rows[2].z * z + transform.Rows[2].w * w,
                transform.Rows[3].x * x + transform.Rows[3].y * y + transform.Rows[3].z * z + transform.Rows[3].w * w);
}

vec4 operator+(vec4 left, vec4 right) { return *left.add(right); }
vec4 operator-(vec4 left, vec4 right) { return *left.subtract(right); }
vec4 operator*(vec4 left, vec4 right) { return *left.multiply(right); }
vec4 operator/(vec4 left, vec4 right) { return *left.divide(right); }

vec4 &vec4::operator+=(vec4 other) { return *add(other); }
vec4 &vec4::operator-=(vec4 other) { return *subtract(other); }
vec4 &vec4::operator*=(vec4 other) { return *multiply(other); }
vec4 &vec4::operator/=(vec4 other) { return *divide(other); }

bool vec4::operator<(vec4 other) const { return x < other.x && y < other.y && z < other.z && w < other.w; }
bool vec4::operator<=(vec4 other) const { return x <= other.x && y <= other.y && z <= other.z && w <= other.w; }
bool vec4::operator>(vec4 other) const { return !(*this < other); }
bool vec4::operator>=(vec4 other) const { return !(*this <= other); }
bool vec4::operator==(vec4 other) { return x == other.x && y == other.y && z == other.z && w == other.w; }
bool vec4::operator!=(vec4 other) { return !(*this == other); }

f32 vec4::dot(vec4 other) { return x * other.x + y * other.y + z * other.z + w * other.w; }
}  // namespace le
