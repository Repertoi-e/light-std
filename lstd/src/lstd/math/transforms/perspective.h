#pragma once

#include "identity.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
struct perspective_helper : non_copyable {
    T FovX, AspectRatio;
    T NearPlane, FarPlane;

    perspective_helper(T fovX, T aspectRatio, T nearPlane, T farPlane)
        : FovX(fovX),
          AspectRatio(aspectRatio),
          NearPlane(nearPlane),
          FarPlane(farPlane) {
    }

    template <typename U, bool MPacked>
    operator mat<U, 4, 4, MPacked>() const {
        mat<U, 4, 4, MPacked> m = zero();
        assert((NearPlane < 0 && FarPlane < NearPlane) || (0 < NearPlane && NearPlane < FarPlane));

        U height = (U) (1 / (T) tan(0.5 * FovX));
        U width  = (U) (height / AspectRatio);
        U fRange = (U) (FarPlane / (NearPlane - FarPlane));
        m(0, 0)  = width;
        m(1, 1)  = height;
        m(2, 2)  = fRange;
        m(2, 3)  = -1;
        m(3, 2)  = fRange * NearPlane;
        return m;
    }
};

// Creates a 3D projection matrix.
template <typename T>
auto perspective(T fov, T aspectRatio, T nearPlane, T farPlane) {
    return perspective_helper(fov, aspectRatio, nearPlane, farPlane);
}

LSTD_END_NAMESPACE
