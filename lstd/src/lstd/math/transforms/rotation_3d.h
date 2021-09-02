#pragma once

#include "../mat_func.h"
#include "../quat.h"
#include "identity.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
struct rotation_3d_axis_helper : non_copyable {
    f32 Angle;
    s64 Axis;

    rotation_3d_axis_helper(const T &angle, s64 axis)
        : Angle(angle),
          Axis(axis) {
    }

    template <typename U, bool MPacked>
    operator mat<U, 4, 4, MPacked>() const {
        mat<U, 4, 4, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, 3, 3, MPacked>() const {
        mat<U, 3, 3, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, 4, 3, MPacked>() const {
        mat<U, 4, 3, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool QPacked>
    operator tquat<U, QPacked>() const;

    template <typename U, s64 R, s64 C_, bool MPacked>
    void set_impl(mat<U, R, C_, MPacked> &m) const {
        T C = (T) cos(Angle);
        T S = (T) sin(Angle);

        assert(0 <= Axis && Axis < 3);

        // Indices according to follow vector order
        if (Axis == 0) {
            // Rotate around X
            m(0, 0) = U(1);
            m(0, 1) = U(0);
            m(0, 2) = U(0);
            m(1, 0) = U(0);
            m(1, 1) = U(C);
            m(1, 2) = U(S);
            m(2, 0) = U(0);
            m(2, 1) = U(-S);
            m(2, 2) = U(C);
        } else if (Axis == 1) {
            // Rotate around Y
            m(0, 0) = U(C);
            m(0, 1) = U(0);
            m(0, 2) = U(-S);
            m(1, 0) = U(0);
            m(1, 1) = U(1);
            m(1, 2) = U(0);
            m(2, 0) = U(S);
            m(2, 1) = U(0);
            m(2, 2) = U(C);
        } else {
            // Rotate around Z
            m(0, 0) = U(C);
            m(0, 1) = U(S);
            m(0, 2) = U(0);
            m(1, 0) = U(-S);
            m(1, 1) = U(C);
            m(1, 2) = U(0);
            m(2, 0) = U(0);
            m(2, 1) = U(0);
            m(2, 2) = U(1);
        }

        // Rest
        For_as(j, range(m.C)) { For_as(i, range(j < 3 ? 3 : 0, m.R)) m(i, j) = U(j == i); }
    }
};

// Rotates around coordinate axis.
// Axis is 0 for X, 1 for Y, 2 for Z and so on...
// Angle of rotation is in radians.
// Positive angles rotate according to the right-hand rule in right-handed coordinate systems
//  (left-handed according to left-hand rule).
template <typename T>
auto rotation_axis(T angle, s64 axis) {
    return rotation_3d_axis_helper(angle, axis);
}

// Rotates around coordinate axis.
// Axis is 0 for X, 1 for Y, 2 for Z and so on...
// Angle of rotation is in radians.
// Positive angles rotate according to the right-hand rule in right-handed coordinate systems
//  (left-handed according to left-hand rule).
template <s64 Axis, typename T>
auto rotation_axis(T angle) {
    return rotation_3d_axis_helper(angle, Axis);
}

// Rotates around the X axis according to the right (left) hand rule in right (left) handed systems.
// Angle of rotation in radians.
template <typename T>
auto rotation_x(T angle) {
    return rotation_axis<0>(angle);
}

// Rotates around the Y axis according to the right (left) hand rule in right (left) handed systems.
// Angle of rotation in radians.
template <typename T>
auto rotation_y(T angle) {
    return rotation_axis<1>(angle);
}

// Rotates around the Z axis according to the right (left) hand rule in right (left) handed systems.
// Angle of rotation in radians.
template <typename T>
auto rotation_z(T angle) {
    return rotation_axis<2>(angle);
}

template <typename T>
struct rotation_3d_tri_axis_helper : non_copyable {
    stack_array<T, 3> Angles;
    stack_array<s64, 3> Axes;

    rotation_3d_tri_axis_helper(stack_array<T, 3> angles, stack_array<s64, 3> axes)
        : Angles(angles),
          Axes(axes) {
    }

    template <typename U, bool MPacked>
    operator mat<U, 4, 4, MPacked>() const {
        mat<U, 4, 4, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, 3, 3, MPacked>() const {
        mat<U, 3, 3, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, 4, 3, MPacked>() const {
        mat<U, 4, 3, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool QPacked>
    operator tquat<U, QPacked>() const;

    template <typename U, s64 R, s64 C, bool MPacked>
    void set_impl(mat<U, R, C, MPacked> &m) const {
        using MatT = mat<U, 3, 3, MPacked>;
        m.get_view < 3, 3 > (0, 0) =
            dot(MatT(rotation_axis(Angles[0], Axes[0])),
                dot(MatT(rotation_axis(Angles[1], Axes[1])), MatT(rotation_axis(Angles[2], Axes[2]))));
        For_as(j, range(m.C)) { For_as(i, range(j < 3 ? 3 : 0, m.R)) m(i, j) = U(j == i); }
    }
};

// Rotates around three axes in succession.
// Axes: 0 for X, 1 for Y and 2 for Z.
// Angles in radians. Each rotation according to the right (and left) hand rule in right (and left) handed systems.
template <s64 FirstAxis, s64 SecondAxis, s64 ThirdAxis, typename T>
auto rotation_axis_3(T angle0, T angle1, T angle2) {
    return rotation_3d_tri_axis_helper(stack_array<T, 3>{angle0, angle1, angle2},
                                       stack_array<s64, 3>{FirstAxis, SecondAxis, ThirdAxis});
}

// Rotation matrix from Euler angles. Rotations are Z-X-Z.
// Each rotation according to the right (and left) hand rule in right (and left) handed systems.
template <typename T>
auto rotation_euler(T z1, T x2, T z3) {
    return rotation_axis_3<2, 0, 2>(z1, x2, z3);
}

template <typename T, bool Packed>
auto rotation_euler(vec<T, 3, Packed> v) {
    return rotation_axis_3<2, 0, 2>(v.x, v.y, v.z);
}

// Rotation matrix from roll-pitch-yaw angles. Rotations are X-Y-Z.
// Each rotation according to the right (and left) hand rule in right (and left) handed systems.
template <typename T>
auto rotation_rpy(T x1, T y2, T z3) {
    return rotation_axis_3<0, 1, 2>(x1, y2, z3);
}

template <typename T, bool Packed>
auto rotation_rpy(vec<T, 3, Packed> v) {
    return rotation_axis_3<0, 1, 2>(v.x, v.y, v.z);
}

template <typename T, bool Packed>
struct rotation_3d_axis_angle_helper : non_copyable {
    const vec<T, 3, Packed> Axis;
    const T Angle;

    rotation_3d_axis_angle_helper(const vec<T, 3, Packed> &axis, T angle)
        : Axis(axis),
          Angle(angle) {
    }

    template <typename U, bool MPacked>
    operator mat<U, 4, 4, MPacked>() const {
        mat<U, 4, 4, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, 3, 3, MPacked>() const {
        mat<U, 3, 3, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, 4, 3, MPacked>() const {
        mat<U, 4, 3, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool QPacked>
    operator tquat<U, QPacked>() const;

    template <typename U, s64 RC, s64 CC, bool MPacked>
    void set_impl(mat<U, RC, CC, MPacked> &m) const {
        assert(is_normalized(Axis));

        T C = (T) cos(Angle);
        T S = (T) sin(Angle);

        // 3x3 rotation sub-matrix
        using RotMat = mat<U, 3, 3, Packed>;
        mat<U, 3, 1, Packed> u(Axis[0], Axis[1], Axis[2]);
        RotMat cross = {U(0), -u(2), u(1), u(2), U(0), -u(0), -u(1), u(0), U(0)};
        RotMat rot   = C * RotMat(identity()) + S * cross + (1 - C) * dot(u, ::T(u));

        // Elements
        For_as(j, range(3)) { For_as(i, range(3)) m(j, i) = rot(i, j); }

        // Rest
        For_as(j, range(m.Width)) { For_as(i, range(j < 3 ? 3 : 0, m.Height)) m(i, j) = U(j == i); }
    }
};

// Rotates around an arbitrary axis.
// Axis of rotation, must be normalized.
// Angle of rotation in radians.
// Right-hand (left-hand) rule is followed in right-handed (left-handed) systems.
template <typename T, bool Vpacked, typename U>
auto rotation_axis_angle(const vec<T, 3, Vpacked> &axis, U angle) {
    return rotation_3d_axis_angle_helper(axis, T(angle));
}

// Determines if the matrix is a proper rotation matrix.
// Proper rotation matrices are orthogonal and have a determinant of +1.
template <typename T, s64 R, s64 C, bool Packed>
bool is_rotation_mat_3d(const mat<T, R, C, Packed> &m) {
    static_assert(R == 3 || R == 4);
    static_assert(C == 3 || C == 4);

    vec<T, 3> r[3] = {
        {m(0, 0), m(0, 1), m(0, 2)},
        {m(1, 0), m(1, 1), m(1, 2)},
        {m(2, 0), m(2, 1), m(2, 2)},
    };

    bool rowsOrthogonal = abs(dot(r[0], r[1])) + abs(dot(r[0], r[2])) + abs(dot(r[1], r[2])) < T(0.0005);
    bool rowsNormalized = is_normalized(r[0]) && is_normalized(r[1]) && is_normalized(r[2]);
    bool properRotation = det(mat<T, 3, 3, Packed>(m.get_view < 3, 3 > (0, 0))) > 0;
    return rowsOrthogonal && rowsNormalized && properRotation;
}

template <typename T>
template <typename U, bool QPacked>
rotation_3d_axis_helper<T>::operator tquat<U, QPacked>() const {
    using QuatT = tquat<U, QPacked>;
    if (Axis == 0) {
        return QuatT(rotation_axis_angle(vec<U, 3, QPacked>(1, 0, 0), Angle));
    }
    if (Axis == 1) {
        return QuatT(rotation_axis_angle(vec<U, 3, QPacked>(0, 1, 0), Angle));
    }
    if (Axis == 2) {
        return QuatT(rotation_axis_angle(vec<U, 3, QPacked>(0, 0, 1), Angle));
    }
    assert(false);
    return {};
}

template <typename T>
template <typename U, bool QPacked>
rotation_3d_tri_axis_helper<T>::operator tquat<U, QPacked>() const {
    using QuatT = tquat<U, QPacked>;
    return qmul(QuatT(rotation_axis(Angles[2], Axes[2])), qmul(QuatT(rotation_axis(Angles[1], Axes[1])), QuatT(rotation_axis(Angles[0], Axes[0]))));
}

template <typename T, bool Packed>
template <typename U, bool QPacked>
rotation_3d_axis_angle_helper<T, Packed>::operator tquat<U, QPacked>() const {
    auto halfAngle = U(Angle) * U(0.5);
    return tquat<U, QPacked>((U) cos(halfAngle), vec<U, 3, QPacked>(Axis) * (U) sin(halfAngle));
}

LSTD_END_NAMESPACE
