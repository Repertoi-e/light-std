#pragma once

#include "identity.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
struct shear_helper : non_copyable {
    T Slope;
    s64 PrincipalAxis;
    s64 ModulatorAxis;

    shear_helper(T slope, s64 principalAxis, s64 modulatorAxis)
        : Slope(slope),
          PrincipalAxis(principalAxis),
          ModulatorAxis(modulatorAxis) {
    }

    template <typename U, s64 R, s64 C, bool MPacked>
    operator mat<U, R, C, MPacked>() const {
        mat<U, R, C, MPacked> m = identity();
        assert(PrincipalAxis != ModulatorAxis);
        assert(ModulatorAxis < R);
        assert(PrincipalAxis < C);
        m(ModulatorAxis, PrincipalAxis) = Slope;
        return m;
    }
};

// Creates a shear matrix.
// Strength of the shear.
// Points are moved along this axis.
// The displacement of points is proportional to this coordinate's value.
template <typename T>
auto shear(T slope, s64 principalAxis, s64 modulatorAxis) {
    return shear_helper(slope, principalAxis, modulatorAxis);
}

LSTD_END_NAMESPACE
