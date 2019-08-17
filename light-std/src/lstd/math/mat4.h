#pragma once

#include "vec3.h"
#include "vec4.h"

LSTD_BEGIN_NAMESPACE

struct mat4 {
    union {
        f32 Elements[4 * 4]{};
        vec4 Rows[4];
    };

    mat4();
    mat4(f32 diagonal);
    mat4(f32 *elements);
    mat4(const vec4 &row0, const vec4 &row1, const vec4 &row2, const vec4 &row3);

    static mat4 IDENTITY();

    mat4 *multiply(const mat4 &other);
    friend mat4 operator*(mat4 left, const mat4 &right);
    mat4 &operator*=(const mat4 &other);

    vec3 multiply(const vec3 &other) const;
    friend vec3 operator*(const mat4 &left, const vec3 &right);

    vec4 multiply(const vec4 &other) const;
    friend vec4 operator*(const mat4 &left, const vec4 &right);

    mat4 *invert();

    vec4 get_column(size_t index) const;
    void set_column(size_t index, const vec4 &column);

    static mat4 orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
    static mat4 perspective(f32 fov, f32 aspectRatio, f32 near, f32 far);
    static mat4 look_at(const vec3 &camera, const vec3 &object, const vec3 &up);

    static mat4 translate(const vec3 &translation);
    static mat4 rotate(f32 angle, const vec3 &axis);
    static mat4 scale(const vec3 &scale);
    static mat4 invert(const mat4 &matrix);

    static mat4 transpose(const mat4 &matrix);
};

LSTD_END_NAMESPACE
