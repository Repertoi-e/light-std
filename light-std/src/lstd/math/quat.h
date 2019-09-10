#pragma once

#include "mat4.h"

#include "../intrin/math.h"

LSTD_BEGIN_NAMESPACE

struct quat {
    f32 x = 0, y = 0, z = 0, w = 1;

    quat() = default;
    quat(f32 x, f32 y, f32 z, f32 w);
    quat(const vec3 &xyz, f32 w);
    quat(const vec4 &vec);
    quat(f32 scalar);

    quat &set_xyz(const vec3 &vec);
    const vec3 get_xyz() const;

    vec3 get_axis() const;
    vec3 to_euler_angles() const;

    quat operator+(const quat &quat) const;
    quat operator-(const quat &quat) const;
    quat operator*(const quat &quat) const;
    quat operator*(f32 scalar) const;
    quat operator/(f32 scalar) const;

    quat &operator+=(const quat &quat) { return *this = *this + quat; }
    quat &operator-=(const quat &quat) { return *this = *this - quat; }
    quat &operator*=(const quat &quat) { return *this = *this * quat; }
    quat &operator*=(f32 scalar) { return *this = *this * scalar; }
    quat &operator/=(f32 scalar) { return *this = *this / scalar; }

    quat operator-() const;
    
	bool operator==(const quat &quaternion) const;
    bool operator!=(const quat &quaternion) const;

    f32 dot(const quat &other) const;
    quat conjugate() const;

    static quat IDENTITY();
    static quat FROM_EULER_ANGLES(const vec3 &angles);

    static vec3 ROTATE(const quat &quat, const vec3 &vec);

    static const quat ROTATION(const vec3 &unitVec0, const vec3 &unitVec1);
    static const quat ROTATION(f32 radians, const vec3 &unitVec);

    static const quat ROTATION_X(f32 radians) {
        f32 angle = radians * 0.5f;
        return quat(SIN(angle), 0.0f, 0.0f, COS(angle));
    }

    static const quat ROTATION_Y(f32 radians) {
        f32 angle = radians * 0.5f;
        return quat(0.0f, SIN(angle), 0.0f, COS(angle));
    }

    static const quat ROTATION_Z(f32 radians) {
        f32 angle = radians * 0.5f;
        return quat(0.0f, 0.0f, SIN(angle), COS(angle));
    }
};

LSTD_END_NAMESPACE
