#include "mat4.h"

#include "../common.h"
#include "../intrin/math.h"

LSTD_BEGIN_NAMESPACE

mat4::mat4() {}

mat4::mat4(f32 diagonal) {
    Elements[0 + 0 * 4] = diagonal;
    Elements[1 + 1 * 4] = diagonal;
    Elements[2 + 2 * 4] = diagonal;
    Elements[3 + 3 * 4] = diagonal;
}

mat4::mat4(f32 *elements) { copy_memory(this->Elements, elements, 4 * 4 * sizeof(f32)); }

mat4::mat4(const vec4 &row0, const vec4 &row1, const vec4 &row2, const vec4 &row3) {
    Rows[0] = row0;
    Rows[1] = row1;
    Rows[2] = row2;
    Rows[3] = row3;
}

mat4 mat4::IDENTITY() { return mat4(1.f); }

mat4 *mat4::multiply(const mat4 &other) {
    f32 data[16];
    For_as(row, range(4)) {
        For_as(col, range(4)) {
            f32 sum = 0.0f;
            For_as(e, range(4)) sum += Elements[e + row * 4] * other.Elements[col + e * 4];
            data[col + row * 4] = sum;
        }
    }
    copy_memory(Elements, data, 4 * 4 * sizeof(f32));
    return this;
}
mat4 operator*(mat4 left, const mat4 &right) { return *left.multiply(right); }
mat4 &mat4::operator*=(const mat4 &other) { return *multiply(other); }

vec3 mat4::multiply(const vec3 &other) const { return other.multiply(*this); }
vec4 mat4::multiply(const vec4 &other) const { return other.multiply(*this); }

vec3 operator*(const mat4 &left, const vec3 &right) { return left.multiply(right); }
vec4 operator*(const mat4 &left, const vec4 &right) { return left.multiply(right); }

mat4 *mat4::invert() {
    f32 temp[16];

    temp[0] = Elements[5] * Elements[10] * Elements[15] - Elements[5] * Elements[11] * Elements[14] -
              Elements[9] * Elements[6] * Elements[15] + Elements[9] * Elements[7] * Elements[14] +
              Elements[13] * Elements[6] * Elements[11] - Elements[13] * Elements[7] * Elements[10];
    temp[4] = -Elements[4] * Elements[10] * Elements[15] + Elements[4] * Elements[11] * Elements[14] +
              Elements[8] * Elements[6] * Elements[15] - Elements[8] * Elements[7] * Elements[14] -
              Elements[12] * Elements[6] * Elements[11] + Elements[12] * Elements[7] * Elements[10];
    temp[8] = Elements[4] * Elements[9] * Elements[15] - Elements[4] * Elements[11] * Elements[13] -
              Elements[8] * Elements[5] * Elements[15] + Elements[8] * Elements[7] * Elements[13] +
              Elements[12] * Elements[5] * Elements[11] - Elements[12] * Elements[7] * Elements[9];
    temp[12] = -Elements[4] * Elements[9] * Elements[14] + Elements[4] * Elements[10] * Elements[13] +
               Elements[8] * Elements[5] * Elements[14] - Elements[8] * Elements[6] * Elements[13] -
               Elements[12] * Elements[5] * Elements[10] + Elements[12] * Elements[6] * Elements[9];
    temp[1] = -Elements[1] * Elements[10] * Elements[15] + Elements[1] * Elements[11] * Elements[14] +
              Elements[9] * Elements[2] * Elements[15] - Elements[9] * Elements[3] * Elements[14] -
              Elements[13] * Elements[2] * Elements[11] + Elements[13] * Elements[3] * Elements[10];
    temp[5] = Elements[0] * Elements[10] * Elements[15] - Elements[0] * Elements[11] * Elements[14] -
              Elements[8] * Elements[2] * Elements[15] + Elements[8] * Elements[3] * Elements[14] +
              Elements[12] * Elements[2] * Elements[11] - Elements[12] * Elements[3] * Elements[10];
    temp[9] = -Elements[0] * Elements[9] * Elements[15] + Elements[0] * Elements[11] * Elements[13] +
              Elements[8] * Elements[1] * Elements[15] - Elements[8] * Elements[3] * Elements[13] -
              Elements[12] * Elements[1] * Elements[11] + Elements[12] * Elements[3] * Elements[9];
    temp[13] = Elements[0] * Elements[9] * Elements[14] - Elements[0] * Elements[10] * Elements[13] -
               Elements[8] * Elements[1] * Elements[14] + Elements[8] * Elements[2] * Elements[13] +
               Elements[12] * Elements[1] * Elements[10] - Elements[12] * Elements[2] * Elements[9];
    temp[2] = Elements[1] * Elements[6] * Elements[15] - Elements[1] * Elements[7] * Elements[14] -
              Elements[5] * Elements[2] * Elements[15] + Elements[5] * Elements[3] * Elements[14] +
              Elements[13] * Elements[2] * Elements[7] - Elements[13] * Elements[3] * Elements[6];
    temp[6] = -Elements[0] * Elements[6] * Elements[15] + Elements[0] * Elements[7] * Elements[14] +
              Elements[4] * Elements[2] * Elements[15] - Elements[4] * Elements[3] * Elements[14] -
              Elements[12] * Elements[2] * Elements[7] + Elements[12] * Elements[3] * Elements[6];
    temp[10] = Elements[0] * Elements[5] * Elements[15] - Elements[0] * Elements[7] * Elements[13] -
               Elements[4] * Elements[1] * Elements[15] + Elements[4] * Elements[3] * Elements[13] +
               Elements[12] * Elements[1] * Elements[7] - Elements[12] * Elements[3] * Elements[5];
    temp[14] = -Elements[0] * Elements[5] * Elements[14] + Elements[0] * Elements[6] * Elements[13] +
               Elements[4] * Elements[1] * Elements[14] - Elements[4] * Elements[2] * Elements[13] -
               Elements[12] * Elements[1] * Elements[6] + Elements[12] * Elements[2] * Elements[5];
    temp[3] = -Elements[1] * Elements[6] * Elements[11] + Elements[1] * Elements[7] * Elements[10] +
              Elements[5] * Elements[2] * Elements[11] - Elements[5] * Elements[3] * Elements[10] -
              Elements[9] * Elements[2] * Elements[7] + Elements[9] * Elements[3] * Elements[6];
    temp[7] = Elements[0] * Elements[6] * Elements[11] - Elements[0] * Elements[7] * Elements[10] -
              Elements[4] * Elements[2] * Elements[11] + Elements[4] * Elements[3] * Elements[10] +
              Elements[8] * Elements[2] * Elements[7] - Elements[8] * Elements[3] * Elements[6];
    temp[11] = -Elements[0] * Elements[5] * Elements[11] + Elements[0] * Elements[7] * Elements[9] +
               Elements[4] * Elements[1] * Elements[11] - Elements[4] * Elements[3] * Elements[9] -
               Elements[8] * Elements[1] * Elements[7] + Elements[8] * Elements[3] * Elements[5];
    temp[15] = Elements[0] * Elements[5] * Elements[10] - Elements[0] * Elements[6] * Elements[9] -
               Elements[4] * Elements[1] * Elements[10] + Elements[4] * Elements[2] * Elements[9] +
               Elements[8] * Elements[1] * Elements[6] - Elements[8] * Elements[2] * Elements[5];

    f32 determinant = Elements[0] * temp[0] + Elements[1] * temp[4] + Elements[2] * temp[8] + Elements[3] * temp[12];
    determinant = 1.f / determinant;

    For(range(16)) Elements[it] = temp[it] * determinant;

    return this;
}

vec4 mat4::get_column(size_t index) const {
    return vec4(Elements[index + 0 * 4], Elements[index + 1 * 4], Elements[index + 2 * 4], Elements[index + 3 * 4]);
}

void mat4::set_column(size_t index, const vec4 &column) {
    Elements[index + 0 * 4] = column.x;
    Elements[index + 1 * 4] = column.y;
    Elements[index + 2 * 4] = column.z;
    Elements[index + 3 * 4] = column.w;
}

mat4 mat4::orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 _near, f32 _far) {
    mat4 result = IDENTITY();

    result.Elements[0 + 0 * 4] = 2.f / (right - left);
    result.Elements[1 + 1 * 4] = 2.f / (top - bottom);
    result.Elements[2 + 2 * 4] = 2.f / (_near - _far);
    result.Elements[3 + 0 * 4] = (left + right) / (left - right);
    result.Elements[3 + 1 * 4] = (bottom + top) / (bottom - top);
    result.Elements[3 + 2 * 4] = (_far + _near) / (_far - _near);

    return result;
}

mat4 mat4::perspective(f32 fov, f32 aspectRatio, f32 _near, f32 _far) {
    mat4 result = IDENTITY();

    f32 q = 1.f / TAN(TO_RAD(0.5f * fov));
    f32 a = q / aspectRatio;

    f32 b = (_near + _far) / (_near - _far);
    f32 c = (2.f * _near * _far) / (_near - _far);

    result.Elements[0 + 0 * 4] = a;
    result.Elements[1 + 1 * 4] = q;
    result.Elements[2 + 2 * 4] = b;
    result.Elements[2 + 3 * 4] = -1.f;
    result.Elements[3 + 2 * 4] = c;

    return result;
}

mat4 mat4::look_at(const vec3 &camera, const vec3 &object, const vec3 &up) {
    mat4 result = IDENTITY();
    vec3 f = (object - camera).normalize();
    vec3 s = f.cross(up.normalize());
    vec3 u = s.cross(f);

    result.Elements[0 + 0 * 4] = s.x;
    result.Elements[0 + 1 * 4] = s.y;
    result.Elements[0 + 2 * 4] = s.z;
    result.Elements[1 + 0 * 4] = u.x;
    result.Elements[1 + 1 * 4] = u.y;
    result.Elements[1 + 2 * 4] = u.z;
    result.Elements[2 + 0 * 4] = -f.x;
    result.Elements[2 + 1 * 4] = -f.y;
    result.Elements[2 + 2 * 4] = -f.z;

    return result * translate(vec3(-camera.x, -camera.y, -camera.z));
}

mat4 mat4::translate(const vec3 &translation) {
    mat4 result = IDENTITY();

    result.Elements[3 + 0 * 4] = translation.x;
    result.Elements[3 + 1 * 4] = translation.y;
    result.Elements[3 + 2 * 4] = translation.z;

    return result;
}

mat4 mat4::rotate(f32 angle, const vec3 &axis) {
    mat4 result = IDENTITY();

    f32 r = TO_RAD(angle);
    f32 c = COS(r);
    f32 s = SIN(r);
    f32 omc = 1.f - c;

    f32 x = axis.x;
    f32 y = axis.y;
    f32 z = axis.z;

    result.Elements[0 + 0 * 4] = x * x * omc + c;
    result.Elements[0 + 1 * 4] = y * x * omc + z * s;
    result.Elements[0 + 2 * 4] = x * z * omc - y * s;

    result.Elements[1 + 0 * 4] = x * y * omc - z * s;
    result.Elements[1 + 1 * 4] = y * y * omc + c;
    result.Elements[1 + 2 * 4] = y * z * omc + x * s;

    result.Elements[2 + 0 * 4] = x * z * omc + y * s;
    result.Elements[2 + 1 * 4] = y * z * omc - x * s;
    result.Elements[2 + 2 * 4] = z * z * omc + c;

    return result;
}

mat4 mat4::scale(const vec3 &scale) {
    mat4 result = IDENTITY();

    result.Elements[0 + 0 * 4] = scale.x;
    result.Elements[1 + 1 * 4] = scale.y;
    result.Elements[2 + 2 * 4] = scale.z;

    return result;
}

mat4 mat4::invert(const mat4 &matrix) {
    mat4 result = matrix;
    return *result.invert();
}

mat4 mat4::transpose(const mat4 &matrix) {
    return mat4(vec4(matrix.Rows[0].x, matrix.Rows[1].x, matrix.Rows[2].x, matrix.Rows[3].x),
                vec4(matrix.Rows[0].y, matrix.Rows[1].y, matrix.Rows[2].y, matrix.Rows[3].y),
                vec4(matrix.Rows[0].z, matrix.Rows[1].z, matrix.Rows[2].z, matrix.Rows[3].z),
                vec4(matrix.Rows[0].w, matrix.Rows[1].w, matrix.Rows[2].w, matrix.Rows[3].w));
}

LSTD_END_NAMESPACE
