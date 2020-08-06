#pragma once

#include "../memory/array.h"
#include "../memory/string_utils.h"
#include "no_init.h"
#include "simd.h"

LSTD_BEGIN_NAMESPACE

// To access swizzlers, use the xx, xy, xyz and similar elements of vectors.
// Swizzlers can be used with assignments, concatenation, casting and constructors.
// To perform arithmetic, cast swizzlers to corresponding vector type.
template <typename VecData, s64... Indices>
struct swizzle {
    using T = typename vec_info<VecData>::type;

    static constexpr s64 IndexTable[] = {Indices...};
    static constexpr s64 Dim = sizeof...(Indices);

    operator vec<T, sizeof...(Indices), false>() const;
    operator vec<T, sizeof...(Indices), true>() const;

    swizzle &operator=(const vec<T, sizeof...(Indices), false> &rhs);
    swizzle &operator=(const vec<T, sizeof...(Indices), true> &rhs);

    template <typename T2, s64... Indices2>
    enable_if_t<sizeof...(Indices) == sizeof...(Indices2), swizzle &> operator=(const swizzle<T2, Indices2...> &rhs) {
        *this = vec<T, sizeof...(Indices2), false>(rhs);
        return *this;
    }

    T &operator[](s64 index) { return ((T *) this)[IndexTable[translate_index(index, Dim)]]; }
    T operator[](s64 index) const { return ((T *) this)[IndexTable[translate_index(index, Dim)]]; }

    template <bool Packed = false>
    const auto to_vec() const {
        return vec<T, Dim, Packed>(*this);
    }

   protected:
    template <s64... Rest>
    enable_if_t<sizeof...(Rest) == 0> assign(const T *) {}

    template <s64 Index, s64... Rest>
    void assign(const T *rhs) {
        ((T *) this)[Index] = *rhs;
        return assign<Rest...>(rhs + 1);
    }
};

template <typename T, s64... Indices>
constexpr s64 swizzle<T, Indices...>::IndexTable[];

template <typename T, s64 Dim, bool Packed>
struct vec_data {
    T Data[Dim];
};

// Small vectors with x,y,z,w members
template <typename T, bool Packed>
struct vec_data<T, 2, Packed> {
    using ST = T;

    vec_data() {}
    vec_data(const vec_data &rhs) {
        for (s64 i = 0; i < 2; ++i) {
            Data[i] = rhs.Data[i];
        }
    }
    vec_data &operator=(const vec_data &rhs) {
        for (s64 i = 0; i < 2; ++i) {
            Data[i] = rhs.Data[i];
        }
        return *this;
    }

    union {
        struct {
            T x, y;
        };

        T Data[2];
#include "swizzle_2.inc"
    };
};

template <typename T, bool Packed>
struct vec_data<T, 3, Packed> {
    using ST = T;

    vec_data() {}
    vec_data(const vec_data &rhs) {
        for (s64 i = 0; i < 3; ++i) {
            Data[i] = rhs.Data[i];
        }
    }
    vec_data &operator=(const vec_data &rhs) {
        for (s64 i = 0; i < 3; ++i) {
            Data[i] = rhs.Data[i];
        }
        return *this;
    }

    union {
        struct {
            T x, y, z;
        };
        T Data[3];
#include "swizzle_3.inc"
    };
};

template <typename T, bool Packed>
struct vec_data<T, 4, Packed> {
    using ST = T;

    vec_data() {}
    vec_data(const vec_data &rhs) {
        for (s64 i = 0; i < 4; ++i) {
            Data[i] = rhs.Data[i];
        }
    }
    vec_data &operator=(const vec_data &rhs) {
        for (s64 i = 0; i < 4; ++i) {
            Data[i] = rhs.Data[i];
        }
        return *this;
    }

    union {
        struct {
            T x, y, z, w;
        };
        T Data[4];
#include "swizzle_4.inc"
    };
};

// Small SIMD fp32 vectors
template <>
struct alignas(16) vec_data<f32, 2, false> {
    using ST = f32;
    using SimdT = simd<ST, 2>;

    vec_data() {}
    vec_data(const vec_data &rhs) { Simd = rhs.Simd; }
    explicit vec_data(SimdT simd) : Simd(simd) {}

    vec_data &operator=(const vec_data &rhs) {
        Simd = rhs.Simd;
        return *this;
    }

    union {
        SimdT Simd;
        struct {
            ST x, y;
        };
        ST Data[2];
#include "swizzle_2.inc"
    };
};

template <>
struct alignas(16) vec_data<f32, 3, false> {
    using ST = f32;
    using SimdT = simd<ST, 4>;

    vec_data() {}
    vec_data(const vec_data &rhs) { Simd = rhs.Simd; }
    explicit vec_data(SimdT simd) : Simd(simd) {}

    vec_data &operator=(const vec_data &rhs) {
        Simd = rhs.Simd;
        return *this;
    }

    union {
        SimdT Simd;
        struct {
            ST x, y, z;
        };
        ST Data[3];
#include "swizzle_3.inc"
    };
};

template <>
struct alignas(16) vec_data<f32, 4, false> {
    using ST = f32;
    using SimdT = simd<ST, 4>;

    vec_data() {}
    vec_data(const vec_data &rhs) { Simd = rhs.Simd; }
    explicit vec_data(SimdT simd) : Simd(simd) {}

    vec_data &operator=(const vec_data &rhs) {
        Simd = rhs.Simd;
        return *this;
    }

    union {
        SimdT Simd;
        struct {
            ST x, y, z, w;
        };
        ST Data[4];
#include "swizzle_4.inc"
    };
};

template <>
struct alignas(16) vec_data<f32, 8, false> {
    using ST = f32;
    using SimdT = simd<f32, 8>;

    vec_data() {}
    vec_data(const vec_data &rhs) { Simd = rhs.Simd; }
    explicit vec_data(SimdT simd) : Simd(simd) {}

    vec_data &operator=(const vec_data &rhs) {
        Simd = rhs.Simd;
        return *this;
    }

    union {
        SimdT Simd;
        f32 Data[8];
    };
};

// Small SIMD fp64 vectors
template <>
struct alignas(16) vec_data<f64, 2, false> {
    using ST = f64;
    using SimdT = simd<ST, 2>;

    vec_data() {}
    vec_data(const vec_data &rhs) { Simd = rhs.Simd; }
    explicit vec_data(SimdT simd) : Simd(simd) {}

    vec_data &operator=(const vec_data &rhs) {
        Simd = rhs.Simd;
        return *this;
    }

    union {
        SimdT Simd;
        struct {
            ST x, y;
        };

        ST Data[2];
#include "swizzle_2.inc"
    };
};

template <>
struct alignas(16) vec_data<f64, 3, false> {
    using ST = f64;
    using SimdT = simd<ST, 4>;

    vec_data() {}
    vec_data(const vec_data &rhs) { Simd = rhs.Simd; }

    explicit vec_data(SimdT simd) : Simd(simd) {}

    vec_data &operator=(const vec_data &rhs) {
        Simd = rhs.Simd;
        return *this;
    }

    union {
        SimdT Simd;
        struct {
            ST x, y, z;
        };

        ST Data[3];
#include "swizzle_3.inc"
    };
};

template <>
struct alignas(16) vec_data<f64, 4, false> {
    using ST = f64;
    using SimdT = simd<ST, 4>;

    vec_data() {}
    vec_data(const vec_data &rhs) { Simd = rhs.Simd; }

    explicit vec_data(SimdT simd) : Simd(simd) {}

    vec_data &operator=(const vec_data &rhs) {
        Simd = rhs.Simd;
        return *this;
    }

    union {
        SimdT Simd;
        struct {
            ST x, y, z, w;
        };
        ST Data[4];
#include "swizzle_4.inc"
    };
};

template <typename T, s64 Dim_, bool Packed_ = false>
struct vec : public vec_data<T, Dim_, Packed_> {
    static_assert(Dim_ >= 1, "Dimension must be a positive integer.");

    using vec_data<T, Dim_, Packed_>::Data;

    struct FromSimd_ {};
    static constexpr FromSimd_ FromSimd = {};

    static constexpr s64 Dim = Dim_;
    static constexpr bool Packed = Packed_;

    vec(no_init_t) : vec_data<T, Dim_, Packed_>() {}

    // Constructs the vector by converting elements
    template <typename U, bool UPacked, typename = enable_if_t<is_convertible_v<U, T>>>
    vec(const vec<U, Dim, UPacked> &other) {
        for (s64 i = 0; i < Dim; ++i) {
            this->Data[i] = (T) other.Data[i];
        }
    }

    // Sets all elements to the same value
    explicit vec(T all) {
        if constexpr (!has_simd_v<vec>) {
            for (auto &v : *this) v = all;
        } else {
            using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
            this->Simd = SimdT::spread(all);
        }
    }

    // Constructs the vector from an array of elements.
    // The number of elements in the array must be at least as the vector's dimension.
    template <typename U, typename = enable_if_t<is_convertible_v<U, T>>>
    vec(const array<U> &data) {
        for (s64 i = 0; i < Dim; ++i) {
            this->Data[i] = (T) data.Data[i];
        }
    }

    template <typename SimdArgT>
    vec(FromSimd_, SimdArgT simd) : vec_data<T, Dim, Packed>(simd) {
        static_assert(has_simd_v<vec>);
    }

    // Creates a homogeneous vector by appending a 1
    template <typename T2, bool Packed2, typename = typename enable_if_t<Dim >= 2>>
    explicit vec(const vec<T2, Dim - 1, Packed2> &rhs) : vec(rhs, 1) {}

    // Truncates last coordinate of homogenous vector to create non-homogeneous
    template <typename T2, bool Packed2>
    explicit vec(const vec<T2, Dim + 1, Packed2> &rhs) : vec(array<T2>((T2 *) rhs.Data, rhs.Dim)) {}

    // Initializes the vector to the given scalar elements.
    // Number of arguments must equal vector dimension.
    template <typename... Scalars, typename = enable_if_t<Dim >= 1 && sizeof...(Scalars) == Dim>,
              typename = enable_if_t<(... && is_convertible_v<Scalars, T>)>>
    vec(Scalars... scalars) : vec(no_init) {
        if constexpr (has_simd_v<vec>) {
            if constexpr (Dim == 3) {
                this->Simd = vec_data<T, 3, Packed>::SimdT::set((T) scalars..., T(0));
            } else {
                this->Simd = vec_data<T, Dim, Packed>::SimdT::set((T) scalars...);
            }
        } else {
            stack_array<T, sizeof...(Scalars)> args = {(T) scalars...};
            for (s64 i = 0; i < Dim; ++i) this->Data[i] = args[i];
        }
    }

    // Initializes the vector by concatenating given scalar, vector or swizzle arguments.
    // Sum of the dimension of arguments must equal vector dimension.
    template <typename... Mixed, typename = enable_if_t<sizeof...(Mixed) >= 1 && sum_of_dims_v<Mixed...> == Dim>,
              typename = enable_if_t<(... || !is_convertible_v<Mixed, T>)>>
    vec(const Mixed &... mixed) {
        assign(0, mixed...);
    }

    template <typename... Args>
    void set(Args &&... args) {
        new (this) vec(((Args &&) args)...);
    }

    T operator[](s64 index) const { return Data[translate_index(index, Dim)]; }
    T &operator[](s64 index) { return Data[translate_index(index, Dim)]; }

    using iterator = T *;
    using const_iterator = const T *;

    iterator begin() { return Data; }
    iterator end() { return Data + Dim; }
    const_iterator begin() const { return Data; }
    const_iterator end() const { return Data + Dim; }

   protected:
    // Get nth element of an argument
    template <typename U>
    struct get_element {
        static U get(const U &u, s64 index) { return u; }
    };

    template <typename T2, s64 D2, bool P2>
    struct get_element<vec<T2, D2, P2>> {
        static T2 get(const vec<T2, D2, P2> &u, s64 index) { return u.Data[index]; }
    };

    template <typename VectorDataU, s64... Indices>
    struct get_element<swizzle<VectorDataU, Indices...>> {
        static auto get(const swizzle<VectorDataU, Indices...> &u, s64 index) {
            return ((typename vec_info<VectorDataU>::type *) &u)[u.IndexTable[index]];
        }
    };

    template <typename Head, typename... Scalars>
    void assign(s64 index, Head &&head, Scalars &&... scalars) {
        if constexpr (is_scalar_v<remove_cvref_t<Head>> && (... && is_scalar_v<remove_cvref_t<Scalars>>) ) {
            Data[index] = (T) head;
            assign(index + 1, ((Scalars &&) scalars)...);
        } else {
            for (s64 i = 0; i < dim_of_v<remove_cvref_t<Head>>; ++i) {
                Data[index] = (T) get_element<remove_cvref_t<Head>>::get(head, i);
                ++index;
            }
            assign(index, ((Scalars &&) scalars)...);
        }
    }

    void assign(s64 index) {
        // terminator
        for (; index < Dim; index++) {
            Data[index] = T(0);
        }
    }
};

template <typename SimdT, s64... Indices>
auto shuffle_reverse(SimdT arg, integer_sequence<s64, Indices...>) {
    return SimdT::template shuffle<Indices...>(arg);
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...>::operator vec<typename swizzle<VectorDataU, Indices...>::T, sizeof...(Indices),
                                               false>() const {
    using DestVecT = vec<T, sizeof...(Indices), false>;

    if constexpr (has_simd_v<VectorDataU> && has_simd_v<DestVecT>) {
        using SourceSimdT = decltype(declval<VectorDataU>().Simd);
        using DestSimdT = decltype(declval<VectorDataU>().Simd);

        constexpr s64 VectorDataDim = vec_info<VectorDataU>::Dim;
        if constexpr (is_same_v<SourceSimdT, DestSimdT>) {
            const auto &sourceSimd = ((const VectorDataU *) this)->Simd;
            if constexpr (sizeof...(Indices) == 3 && VectorDataDim == 3 && VectorDataDim == 4) {
                return {
                    DestVecT::FromSimd,
                    shuffle_reverse(sourceSimd,
                                    typename reverse_integer_sequence<integer_sequence<s64, Indices..., 3>>::type{})};
            } else if constexpr (sizeof...(Indices) == 4 && VectorDataDim == 3 && VectorDataDim == 4) {
                return {DestVecT::FromSimd,
                        shuffle_reverse(sourceSimd,
                                        typename reverse_integer_sequence<integer_sequence<s64, Indices...>>::type{})};
            } else if constexpr (sizeof...(Indices) == 2 && VectorDataDim == 2) {
                return {DestVecT::FromSimd,
                        shuffle_reverse(sourceSimd,
                                        typename reverse_integer_sequence<integer_sequence<s64, Indices...>>::type{})};
            }
        }
    }
    return DestVecT(((typename vec_info<VectorDataU>::type *) this)[Indices]...);
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...>::operator vec<typename swizzle<VectorDataU, Indices...>::T, sizeof...(Indices), true>()
    const {
    return vec<T, sizeof...(Indices), true>(Data[Indices]...);
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...> &swizzle<VectorDataU, Indices...>::operator=(
    const vec<T, sizeof...(Indices), false> &rhs) {
    if (((typename vec_info<VectorDataU>::type *) this) != rhs.Data) {
        assign<Indices...>(rhs.Data);
    } else {
        vec<T, sizeof...(Indices), false> temp = rhs;
        *this = temp;
    }
    return *this;
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...> &swizzle<VectorDataU, Indices...>::operator=(
    const vec<T, sizeof...(Indices), true> &rhs) {
    if (((typename vec_info<VectorDataU>::type *) this) != rhs.Data) {
        assign<Indices...>(rhs.Data);
    } else {
        vec<T, sizeof...(Indices), false> temp = rhs;
        *this = temp;
    }
    return *this;
}

template <typename T, bool Packed = false>
using vec2 = vec<T, 2, Packed>;

template <typename T, bool Packed = false>
using vec3 = vec<T, 3, Packed>;

template <typename T, bool Packed = false>
using vec4 = vec<T, 4, Packed>;

using v2 = vec2<f32>;
using v3 = vec3<f32>;
using v4 = vec4<f32>;

LSTD_END_NAMESPACE
