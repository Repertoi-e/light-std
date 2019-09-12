#include "quat.h"

LSTD_BEGIN_NAMESPACE

quat::quat(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}

quat::quat(const vec4 &vec) : x(vec.x), y(vec.y), z(vec.z), w(vec.w) {}

quat::quat(f32 scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}

quat::quat(const vec3 &xyz, f32 w) {
    this->set_xyz(xyz);
    this->w = w;
}

quat &quat::set_xyz(const vec3 &vec) {
    x = vec.x;
    y = vec.y;
    z = vec.z;
    return *this;
}

const vec3 quat::get_xyz() const { return vec3(x, y, z); }

vec3 quat::get_axis() const {
    f32 x = 1.0f - w * w;
    if (x < 0.0000001f) return vec3(1, 0, 0);

    f32 x2 = x * x;
    return get_xyz() / x2;
}

vec3 quat::to_euler_angles() const {
    return vec3(ATAN2(2 * x * w - 2 * y * z, 1 - 2 * x * x - 2 * z * z),
                ATAN2(2 * y * w - 2 * x * z, 1 - 2 * y * y - 2 * z * z), ASIN(2 * x * y + 2 * z * w));
}

quat quat::operator+(const quat &quaternion) const {
    return quat(x + quaternion.x, y + quaternion.y, z + quaternion.z, w + quaternion.w);
}

quat quat::operator-(const quat &quaternion) const {
    return quat(x - quaternion.x, y - quaternion.y, z - quaternion.z, w - quaternion.w);
}

quat quat::operator*(f32 scalar) const { return quat(x * scalar, y * scalar, z * scalar, w * scalar); }

quat quat::operator/(f32 scalar) const { return quat(x / scalar, y / scalar, z / scalar, w / scalar); }

quat quat::operator-() const { return quat(-x, -y, -z, -w); }

bool quat::operator==(const quat &quat) const {
    return (x == quat.x) && (y == quat.y) && (z == quat.z) && (w == quat.w); // @TODO: ABS(x - quat.x) < Epsilon (and for other math types!)
}

bool quat::operator!=(const quat &quat) const { return !(*this == quat); }

static f32 norm(const quat &quaternion) {
    f32 result;
    result = (quaternion.x * quaternion.x);
    result = (result + (quaternion.y * quaternion.y));
    result = (result + (quaternion.z * quaternion.z));
    result = (result + (quaternion.w * quaternion.w));
    return result;
}

f32 length(const quat &quaternion) { return SQRT(norm(quaternion)); }

quat normalize(const quat &quaternion) { return quaternion * INV_SQRT(norm(quaternion)); }

quat quat::operator*(const quat &quat) const {
    return normalize(::quat((((w * quat.x) + (x * quat.w)) + (y * quat.z)) - (z * quat.y),
                            (((w * quat.y) + (y * quat.w)) + (z * quat.x)) - (x * quat.z),
                            (((w * quat.z) + (z * quat.w)) + (x * quat.y)) - (y * quat.x),
                            (((w * quat.w) - (x * quat.x)) - (y * quat.y)) - (z * quat.z)));
}

quat quat::conjugate() const { return quat(-x, -y, -z, w); }

f32 quat::dot(const quat &other) const {
    f32 result = (x * other.x);
    result = (result + (y * other.y));
    result = (result + (z * other.z));
    result = (result + (w * other.w));
    return result;
}

quat quat::IDENTITY() { return quat(0.0f, 0.0f, 0.0f, 1.0f); }

quat quat::FROM_EULER_ANGLES(const vec3 &angles) {
    quat pitch(vec3(1.0, 0.0, 0.0), angles.x);
    quat yaw(vec3(0.0, 1.0, 0.0), angles.y);
    quat roll(vec3(0.0, 0.0, 1.0), angles.z);
    return pitch * yaw * roll;
}

vec3 quat::ROTATE(const quat &quat, const vec3 &vec) {
    f32 tmpX, tmpY, tmpZ, tmpW;
    tmpX = (((quat.w * vec.x) + (quat.y * vec.z)) - (quat.z * vec.y));
    tmpY = (((quat.w * vec.y) + (quat.z * vec.x)) - (quat.x * vec.z));
    tmpZ = (((quat.w * vec.z) + (quat.x * vec.y)) - (quat.y * vec.x));
    tmpW = (((quat.x * vec.x) + (quat.y * vec.y)) + (quat.z * vec.z));
    return vec3(((((tmpW * quat.x) + (tmpX * quat.w)) - (tmpY * quat.z)) + (tmpZ * quat.y)),
                ((((tmpW * quat.y) + (tmpY * quat.w)) - (tmpZ * quat.x)) + (tmpX * quat.z)),
                ((((tmpW * quat.z) + (tmpZ * quat.w)) - (tmpX * quat.y)) + (tmpY * quat.x)));
}

const quat quat::ROTATION(const vec3 &unitVec0, const vec3 &unitVec1) {
    f32 cosHalfAngleX2, recipCosHalfAngleX2;
    cosHalfAngleX2 = SQRT((2.0f * (1.0f + unitVec0.dot(unitVec1))));
    recipCosHalfAngleX2 = (1.0f / cosHalfAngleX2);
    return quat((unitVec0.cross(unitVec1) * recipCosHalfAngleX2), (cosHalfAngleX2 * 0.5f));
}

const quat quat::ROTATION(f32 radians, const vec3 &unitVec) {
    f32 angle = radians * 0.5f;
    return quat((unitVec * SIN(angle)), COS(angle));
}

LSTD_END_NAMESPACE
