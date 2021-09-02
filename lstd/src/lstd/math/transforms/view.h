#pragma once

#include "identity.h"

LSTD_BEGIN_NAMESPACE

template <typename T, s64 Dim, bool Packed>
struct view_helper : non_copyable {
    using VectorT = vec<T, Dim, Packed>;

    VectorT Eye;
    VectorT Target;
    stack_array<VectorT, s64(Dim - 2)> Bases;
    stack_array<bool, Dim> FlipAxes;

    view_helper(const VectorT &eye,
                const VectorT &target,
                const stack_array<VectorT, s64(Dim - 2)> &bases,
                const stack_array<bool, Dim> &flipAxes)
        : Eye(eye),
          Target(target),
          Bases(bases),
          FlipAxes(flipAxes) {
    }

    template <typename U, bool MPacked>
    operator mat<U, Dim + 1, Dim + 1, MPacked>() const {
        mat<U, Dim + 1, Dim + 1, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, bool MPacked>
    operator mat<U, Dim + 1, Dim, MPacked>() const {
        mat<U, Dim + 1, Dim, MPacked> m;
        set_impl(m);
        return m;
    }

    template <typename U, s64 R, s64 C, bool MPacked>
    void set_impl(mat<U, R, C, MPacked> &m) const {
        stack_array<VectorT, Dim> columns;

        stack_array<const VectorT *, Dim - 1> crossTable = {};
        For(range(Bases.Count)) crossTable[it] = &Bases[it];

        crossTable[-1] = &columns[Dim - 1];

        // Calculate columns of the rotation matrix
        s64 j      = Dim - 1;
        columns[j] = normalize(Eye - Target); // Right-handed: camera look towards -Z
        do {
            --j;
            columns[Dim - j - 2] = normalize(cross(crossTable));

            // Shift bases
            For_as(s, range(j)) crossTable[s] = crossTable[s + 1];
            crossTable[j] = &columns[Dim - j - 2];
        } while (j > 0);

        // Flip columns
        For(range(Dim)) {
            if (FlipAxes[it]) columns[it] *= -T(1);
        }

        // Copy columns to matrix
        For_as(i, range(Dim)) { For_as(j, range(Dim)) m(i, j) = columns[j][i]; }

        // Calculate translation of the matrix
        For(range(Dim)) { m(Dim, it) = -dot(Eye, columns[it]); }

        // Clear additional elements
        constexpr s64 auxDim = R < C ? R : C;
        if (auxDim > Dim) {
            For(range(Dim)) { m(it, auxDim - 1) = 0; }
            m(Dim, auxDim - 1) = 1;
        }
    }
};

/// Creates a general, n-dimensional camera look-at matrix.
// _eye_ represents the camera's position
// _target_ represents the camera's target
// _bases_ - basis vectors fixing the camera's orientation
// _flipAxis_ - set any element to true to flip an axis in camera space
//
// The camera looks down the vector going from _eye_ to _target_, but it can still rotate around that vector.
// To fix the rotation, an "up" vector must be provided in 3 dimensions. In higher dimensions, we need multiple up
// vectors. Unfortunately I can't remember how these basis vectors are used, but they are orthogonalized to each-other
// and to the look vector. I can't remember the order of orthogonalization.
template <typename T, s64 Dim, bool Packed, s64 BaseDim, s64 FlipDim>
auto look_at(const vec<T, Dim, Packed> &eye,
             const vec<T, Dim, Packed> &target,
             const stack_array<vec<T, Dim, Packed>, BaseDim> &bases,
             const stack_array<bool, FlipDim> &flipAxes) {
    static_assert(BaseDim == Dim - 2, "You must provide 2 fewer bases than the dimension of the transform.");
    static_assert(Dim == FlipDim, "You must provide the same number of flips as the dimension of the transform.");
    return view_helper<T, Dim, Packed>(eye, target, bases, flipAxes);
}

// Creates a 2D look-at matrix.
// _eye_ - the camera's position
// _target_ - the camera's target
// _positiveYForward_ - true if the camera looks towards +Y in camera space, false if -Y
// _flipX_ - true to flip X in camera space
template <typename T, bool Packed>
auto look_at(const vec<T, 2, Packed> &eye, const vec<T, 2, Packed> &target, bool positiveYForward, bool flipX) {
    return look_at(eye, target, stack_array<vec<T, 2, Packed>, 0>{}, stack_array<bool, 2>{flipX, positiveYForward});
}

// Creates a 3D look-at matrix.
// eye - the camera's position.
// target - the camera's target.
// up - up direction in world space.
// positiveZForward" - true if the camera looks towards +Z in camera space, false if -Z.
// flipX - true to flip X in camera space.
// flipY - true to flip Y in camera space.
// The camera space X is selected to be orthogonal to both the look direction and the _up_ vector.
// Afterwards, the _up_ vector is re-orthogonalized to the camera-space Z and X vectors.
template <typename T, bool Packed>
auto look_at(const vec<T, 3, Packed> &eye, const vec<T, 3, Packed> &target, const vec<T, 3, Packed> &up, bool positiveZForward, bool flipX, bool flipY) {
    return look_at(eye, target, stack_array<vec<T, 3, Packed>, 1>{up}, stack_array<bool, 3>{flipX, flipY, positiveZForward});
}

LSTD_END_NAMESPACE
