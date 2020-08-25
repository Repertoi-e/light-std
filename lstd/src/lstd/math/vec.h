#pragma once

#include "../memory/array.h"
#include "../memory/string_utils.h"
#include "no_init.h"
#include "simd.h"
#include "swizzle_1.inl"
#include "swizzle_2.inl"
#include "swizzle_3.inl"
#include "swizzle_4.inl"

LSTD_BEGIN_NAMESPACE

//
// Helpers for accesing static information about vectors
//
template <typename T>
struct vec_info_helper {
    static constexpr bool IS_VEC = false;
};

template <typename DataT, s64 Dim, bool Packed>
struct vec_info_helper<vec<DataT, Dim, Packed>> {
    static constexpr bool IS_VEC = true;

    using T = DataT;
    static constexpr s64 DIM = Dim;
    static constexpr bool PACKED = Packed;
};

template <typename DataT, s64 Dim, bool Packed>
struct vec_info_helper<vec_data<DataT, Dim, Packed>> {
    static constexpr bool IS_VEC = true;

    using T = DataT;
    static constexpr s64 DIM = Dim;
    static constexpr bool PACKED = Packed;
};

template <typename T>
struct vec_info : public vec_info_helper<types::decay_t<T>> {};

template <typename T>
concept has_simd = requires(T a) {
    vec_info<T>{}.IS_VEC;
    a.Simd;
};

// To access swizzlers, use the xx, xy, xyz and similar elements of vectors.
// Swizzlers can be used with assignments, concatenation, casting and constructors.
// To perform arithmetic, cast swizzlers to corresponding vector type.
template <typename VecData, s64... Indices>
struct swizzle {
    using T = typename vec_info<VecData>::T;

    inline static constexpr s64 IndexTable[] = {Indices...};
    static constexpr s64 Dim = sizeof...(Indices);

    operator vec<T, sizeof...(Indices), false>() const;
    operator vec<T, sizeof...(Indices), true>() const;

    swizzle &operator=(const vec<T, sizeof...(Indices), false> &rhs);
    swizzle &operator=(const vec<T, sizeof...(Indices), true> &rhs);

    template <typename T2, s64... Indices2>
    requires(sizeof...(Indices) == sizeof...(Indices2)) swizzle &operator=(const swizzle<T2, Indices2...> &rhs) {
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
    requires(sizeof...(Rest) == 0) void assign(const T *) {}

    template <s64 Index, s64... Rest>
    void assign(const T *rhs) {
        ((T *) this)[Index] = *rhs;
        return assign<Rest...>(rhs + 1);
    }
};

//
// Gets the dimension of a vec or a swizzle
//
template <typename U, s32 Along = 0>
struct dim_of {
    static constexpr s64 value = 1;
};

template <typename T, s64 Dim, bool Packed>
struct dim_of<vec<T, Dim, Packed>, 0> {
    static constexpr s64 value = Dim;
};

template <typename T, s64... Indices>
struct dim_of<swizzle<T, Indices...>> {
    static constexpr s64 value = sizeof...(Indices);
};

template <typename T>
constexpr s64 dim_of_v = dim_of<T>::value;

//
// vec_data is a base class of vec
//

template <typename T, s64 Dim, bool Packed>
struct vec_data {
    T Data[Dim];
};

//
// Small vectors with x, y, z, w members
// VEC_DATA_ macro (defined in swizzle_*.h) includes Data array of proper dimension and x, (y, z, w) members
//
#define VEC_DATA_DEF(dim)                          \
    template <typename T, bool Packed>             \
    struct vec_data<T, dim, Packed> {              \
        using ST = T;                              \
                                                   \
        vec_data() {}                              \
        vec_data(const vec_data &rhs) {            \
            for (s64 i = 0; i < dim; ++i) {        \
                Data[i] = rhs.Data[i];             \
            }                                      \
        }                                          \
        vec_data &operator=(const vec_data &rhs) { \
            for (s64 i = 0; i < dim; ++i) {        \
                Data[i] = rhs.Data[i];             \
            }                                      \
            return *this;                          \
        }                                          \
                                                   \
        union {                                    \
            VEC_DATA_##dim                         \
        };                                         \
    }

VEC_DATA_DEF(2);
VEC_DATA_DEF(3);
VEC_DATA_DEF(4);
#undef VEC_DATA_DEF

#define VEC_DATA_8 ST Data[8];

//
// Small SIMD f32 vectors
// VEC_DATA_ macro (defined in swizzle_*.h) includes Data array of proper dimension and x, (y, z, w) members
//
#define VEC_DATA_DEF(type, dim, simdDim)                   \
    template <>                                            \
    struct alignas(16) vec_data<type, dim, false> {        \
        using ST = type;                                   \
        using SimdT = simd<ST, simdDim>;                   \
                                                           \
        vec_data() {}                                      \
        vec_data(const vec_data &rhs) { Simd = rhs.Simd; } \
        explicit vec_data(SimdT simd) : Simd(simd) {}      \
                                                           \
        vec_data &operator=(const vec_data &rhs) {         \
            Simd = rhs.Simd;                               \
            return *this;                                  \
        }                                                  \
                                                           \
        union {                                            \
            SimdT Simd;                                    \
            VEC_DATA_##dim                                 \
        };                                                 \
    }
VEC_DATA_DEF(f32, 2, 2);
VEC_DATA_DEF(f32, 3, 4);  // We turn on SIMD for vectors with three components (but we treat them as having 4)
VEC_DATA_DEF(f32, 4, 4);
VEC_DATA_DEF(f32, 8, 8);

// Small SIMD f64 vectors
VEC_DATA_DEF(f64, 2, 2);
VEC_DATA_DEF(f64, 3, 4);  // We turn on SIMD for vectors with three components (but we treat them as having 4)
VEC_DATA_DEF(f64, 4, 4);
VEC_DATA_DEF(f64, 8, 8);

template <typename T, s64 Dim_, bool Packed_ = false>
struct vec : public vec_data<T, Dim_, Packed_> {
    static_assert(Dim_ >= 1, "Dimension must be a positive integer >= 1.");

    using vec_data<T, Dim_, Packed_>::Data;

    struct FromSimd_ {};
    static constexpr FromSimd_ FromSimd = {};

    static constexpr s64 Dim = Dim_;
    static constexpr bool Packed = Packed_;

    vec() {
        for (s64 i = 0; i < Dim; ++i) {
            this->Data[i] = T(0);
        }
    }

    // :MathTypesNoInit By default we zero-init but you can call a special constructor with the value no_init which doesn't initialize the object
    vec(no_init_t) : vec_data<T, Dim_, Packed_>() {}

    // Constructs the vector by converting elements
    template <typename U, bool UPacked>
    requires(types::is_convertible_v<U, T>) vec(const vec<U, Dim, UPacked> &other) {
        for (s64 i = 0; i < Dim; ++i) {
            this->Data[i] = (T) other.Data[i];
        }
    }

    // Sets all elements to the same value
    explicit vec(T all) {
        if constexpr (!has_simd<vec>) {
            for (auto &v : *this) v = all;
        } else {
            using SimdT = decltype(vec_data<T, Dim, Packed>::Simd);
            this->Simd = SimdT::spread(all);
        }
    }

    // Constructs the vector from an array of elements.
    // The number of elements in the array must be at least as the vector's dimension.
    template <typename U>
    requires(types::is_convertible_v<U, T>) vec(const array_view<U> &data) {
        for (s64 i = 0; i < Dim; ++i) {
            this->Data[i] = (T) data.Data[i];
        }
    }

    template <typename SimdArgT>
    vec(FromSimd_, SimdArgT simd) : vec_data<T, Dim, Packed>(simd) {
        static_assert(has_simd<vec>);
    }

    // Creates a homogeneous vector by appending a 1
    template <typename T2, bool Packed2>
    requires(Dim >= 2) explicit vec(const vec<T2, Dim - 1, Packed2> &rhs) : vec(rhs, 1) {}

    // Truncates last coordinate of homogenous vector to create non-homogeneous
    template <typename T2, bool Packed2>
    explicit vec(const vec<T2, Dim + 1, Packed2> &rhs) : vec(array_view<T2>((T2 *) rhs.Data, rhs.Dim)) {}

    // Initializes the vector to the given scalar elements.
    // Number of arguments must equal vector dimension.
    template <typename... Scalars>
    requires((Dim >= 1) && (sizeof...(Scalars) == Dim) && ((... && types::is_convertible_v<Scalars, T>) )) vec(Scalars... scalars) : vec(no_init) {
        if constexpr (has_simd<vec>) {
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
    template <typename... Mixed>
    requires((sizeof...(Mixed) >= 1) && ((dim_of_v<Mixed> + ...) == Dim) && ((... || !types::is_convertible_v<Mixed, T>) )) vec(const Mixed &... mixed) {
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
            return ((typename vec_info<VectorDataU>::T *) &u)[u.IndexTable[index]];
        }
    };

    template <typename Head, typename... Scalars>
    void assign(s64 index, Head &&head, Scalars &&... scalars) {
        if constexpr (types::is_scalar_v<types::remove_cvref_t<Head>> && (... && types::is_scalar_v<types::remove_cvref_t<Scalars>>) ) {
            Data[index] = (T) head;
            assign(index + 1, ((Scalars &&) scalars)...);
        } else {
            for (s64 i = 0; i < dim_of_v<types::remove_cvref_t<Head>>; ++i) {
                Data[index] = (T) get_element<types::remove_cvref_t<Head>>::get(head, i);
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
auto shuffle_reverse(SimdT arg, types::integer_sequence<s64, Indices...>) {
    return SimdT::template shuffle<Indices...>(arg);
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...>::operator vec<typename swizzle<VectorDataU, Indices...>::T, sizeof...(Indices), false>() const {
    using DestVecT = vec<T, sizeof...(Indices), false>;

    if constexpr (has_simd<VectorDataU> && has_simd<DestVecT>) {
        using SourceSimdT = decltype(types::declval<VectorDataU>().Simd);
        using DestSimdT = decltype(types::declval<VectorDataU>().Simd);

        constexpr s64 VectorDataDim = vec_info<VectorDataU>::DIM;
        if constexpr (types::is_same_v<SourceSimdT, DestSimdT>) {
            auto &sourceSimd = ((VectorDataU *) this)->Simd;
            if constexpr (sizeof...(Indices) == 3 && VectorDataDim == 3 && VectorDataDim == 4) {
                return {DestVecT::FromSimd, shuffle_reverse(sourceSimd, typename types::reverse_integer_sequence<types::integer_sequence<s64, Indices..., 3>>::type{})};
            } else if constexpr (sizeof...(Indices) == 4 && VectorDataDim == 3 && VectorDataDim == 4) {
                return {DestVecT::FromSimd, shuffle_reverse(sourceSimd, typename types::reverse_integer_sequence<types::integer_sequence<s64, Indices...>>::type{})};
            } else if constexpr (sizeof...(Indices) == 2 && VectorDataDim == 2) {
                return {DestVecT::FromSimd, shuffle_reverse(sourceSimd, typename types::reverse_integer_sequence<types::integer_sequence<s64, Indices...>>::type{})};
            }
        }
    }
    return DestVecT(((typename vec_info<VectorDataU>::T *) this)[Indices]...);
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...>::operator vec<typename swizzle<VectorDataU, Indices...>::T, sizeof...(Indices), true>() const {
    return vec<T, sizeof...(Indices), true>(Data[Indices]...);
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...> &swizzle<VectorDataU, Indices...>::operator=(const vec<T, sizeof...(Indices), false> &rhs) {
    if (((typename vec_info<VectorDataU>::T *) this) != rhs.Data) {
        assign<Indices...>(rhs.Data);
    } else {
        vec<T, sizeof...(Indices), false> temp = rhs;
        *this = temp;
    }
    return *this;
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...> &swizzle<VectorDataU, Indices...>::operator=(const vec<T, sizeof...(Indices), true> &rhs) {
    if (((typename vec_info<VectorDataU>::T *) this) != rhs.Data) {
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
