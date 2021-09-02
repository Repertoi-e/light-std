#pragma once

#include "../memory/array.h"
#include "../memory/string_utils.h"
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

template <typename T_, s64 Dim, bool Packed>
struct vec_info_helper<vec<T_, Dim, Packed>> {
    static constexpr bool IS_VEC = true;

    using T = T_;
    static constexpr s64 DIM     = Dim;
    static constexpr bool PACKED = Packed;
};

template <typename T_, s64 Dim, bool Packed>
struct vec_info_helper<vec_data<T_, Dim, Packed>> {
    static constexpr bool IS_VEC = true;

    using T = T_;
    static constexpr s64 DIM     = Dim;
    static constexpr bool PACKED = Packed;
};

template <typename T>
struct vec_info : public vec_info_helper<types::remove_cvref_t<T>> {
};

template <typename T>
concept has_simd = requires(T t)
{
    vec_info<T>::IS_VEC == true;
    t.Simd;
};

template <typename T>
concept any_vec = vec_info<T>::IS_VEC;

// To access swizzlers, use the xx, xy, xyz and similar elements of vectors.
// Swizzlers can be used with assignments, concatenation, casting and constructors.
// To perform arithmetic, cast swizzlers to corresponding vector type.
template <typename VecData, s64... Indices>
struct swizzle {
    static_assert(any_vec<VecData>, "Swizzle expects a valid vector data type");

    using T = typename vec_info<VecData>::T;

    inline static constexpr s64 INDEX_TABLE[] = {Indices...};
    static constexpr s64 DIM                  = sizeof...(Indices);

    operator vec<T, sizeof...(Indices), false>() const;
    operator vec<T, sizeof...(Indices), true>() const;

    swizzle &operator=(const vec<T, sizeof...(Indices), false> &rhs);
    swizzle &operator=(const vec<T, sizeof...(Indices), true> &rhs);

    template <typename T2, s64... Indices2>
        requires(sizeof...(Indices) == sizeof...(Indices2))
    swizzle &operator=(const swizzle<T2, Indices2...> &rhs) {
        *this = vec<T, sizeof...(Indices2), false>(rhs);
        return *this;
    }

    T &operator[](s64 index) { return ((T *) this)[INDEX_TABLE[translate_index(index, DIM)]]; }
    T operator[](s64 index) const { return ((T *) this)[INDEX_TABLE[translate_index(index, DIM)]]; }

    template <bool Packed = false>
    const auto to_vec() const { return vec<T, DIM, Packed>(*this); }

private:
    template <s64... Rest>
        requires(sizeof...(Rest) == 0)
    void assign(const T *) {
    }

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

VEC_DATA_DEF(f32, 3, 4); // :SimdForVec3: We turn on SIMD for vectors with three components (but we treat them as having 4)
VEC_DATA_DEF(f32, 4, 4);

VEC_DATA_DEF(f32, 8, 8);

// Small SIMD f64 vectors
VEC_DATA_DEF(f64, 2, 2);

VEC_DATA_DEF(f64, 3, 4); // :SimdForVec3: We turn on SIMD for vectors with three components (but we treat them as having 4)
VEC_DATA_DEF(f64, 4, 4);

VEC_DATA_DEF(f64, 8, 8);

template <typename T_, s64 Dim, bool Packed = false>
struct vec : public vec_data<T_, Dim, Packed> {
    static_assert(Dim >= 1, "Dimension must be >= 1");

    using T = T_;
    static constexpr s64 DIM     = Dim;
    static constexpr bool PACKED = Packed;

    // :CodeReusability: Automatically generates ==, !=, <, <=, >, >=, compare_*, find_*, has functions etc.. take a look at "array_like.h"
    static constexpr s64 Count = DIM;

    // :MathTypesNoInit By default we don't init (to save on performance) but you can call a constructor with a scalar value of 0 to zero-init.
    vec()
        : vec_data<T, DIM, PACKED>() {
    }

    // Sets all elements to the same value
    explicit vec(T all) {
        if constexpr (!has_simd<vec>) {
            For(*this) it = all;
        } else {
            using SimdT = decltype(this->Simd);
            this->Simd = SimdT::spread(all);
        }
    }

    // Convertible if the vectors are of the same size and the elements are convertible
    template <any_vec Vec>
        requires(types::is_convertible<T, vec_info<Vec>::T> && (vec_info<Vec>::DIM == DIM))
    vec(const Vec &v) {
        For(range(DIM)) this->Data[it] = T(v.Data[it]);
    }

    // Constructs the vector from an array of elements.
    // The number of elements in the array must be at least as the vector's dimension.
    template <typename U>
        requires(types::is_convertible<U, T>)
    vec(const array<U> &data) {
        assert(DIM <= data.Count);
        for (s64 i = 0; i < DIM; ++i) {
            this->Data[i] = T(data.Data[i]);
        }
    }

    // Creates a homogeneous vector by appending a 1
    template <any_vec Vec>
        requires(DIM >= 2 && (Vec::DIM == DIM - 1))
    explicit vec(const Vec &v)
        : vec(v, 1) {
    }

    // Truncates last coordinate of homogenous vector to create non-homogeneous
    template <any_vec Vec>
        requires(Vec::DIM == DIM + 1)
    explicit vec(const Vec &v)
        : vec(array<typename Vec::T>((typename Vec::T *) v.Data, v.DIM)) {
    }

    // Initializes the vector from the given elements - either just scalars or mixed with vectors and swizzles.
    //   e.g. vec3(pos.xy, 0) or just vec3(pos, 0).
    // In the case it is just scalars
    //   e.g. vec3(1, 4, 9) we can try to use SIMD for faster assignment.
    //
    // The sum of the dimensions of the arguments must be equal to the dimension of the vector (scalars have one dimension).
    // We don't support arrays here - they get treated as scalars and fail because they might not be convertible to the type of the vector.
    template <typename... Args>
        requires((sizeof...(Args) >= 1) && ((dim_of_v<Args> + ...) == DIM))
    vec(const Args &... args) {
        // If not all are convertible to T we treat as mixed.
        if constexpr ((!types::is_convertible<Args, T> || ...)) {
            assign_from_mixed(0, args...);
        } else {
            if constexpr (has_simd<vec>) {
                // :SimdForVec3: We support SIMD for vectors with a dimension of 3 and we treat them as having 4 components
                if constexpr (DIM == 3) {
                    this->Simd = vec_data<T, DIM, PACKED>::SimdT::set(T(args)..., T(0));
                } else {
                    this->Simd = vec_data<T, DIM, PACKED>::SimdT::set(T(args)...);
                }
            } else {
                // We did this before but this copies all the arguments to the stack first..
                //
                //    stack_array<T, sizeof...(Args)> args = {T(args)...};
                //    For(range(DIM)) this->Data[it] = args[it];
                //
                // assign_from_mixed might get optimized better (we are passing just scalars here).
                assign_from_mixed(0, args...);
            }
        }
    }

    // We use this type to validate constructing a vector from SIMD
    struct from_simd_t {
    };

    static constexpr from_simd_t FROM_SIMD = {};

    template <typename SimdArgT>
    vec(from_simd_t, SimdArgT simd)
        : vec_data<T, DIM, PACKED>(simd) { static_assert(has_simd<vec>); }

    //
    // Iterators:
    //
    using iterator = T *;
    using const_iterator = const T *;

    iterator begin() { return this->Data; }
    iterator end() { return this->Data + DIM; }
    const_iterator begin() const { return this->Data; }
    const_iterator end() const { return this->Data + DIM; }

    //
    // Operators:
    //

    // We support negative indexing, -1 means the last element (DIM - 1) and so on..
    T operator[](s64 index) const { return this->Data[translate_index(index, DIM)]; }
    T &operator[](s64 index) { return this->Data[translate_index(index, DIM)]; }

private:
    template <typename Head, typename... Rest>
    void assign_from_mixed(s64 index, const Head &head, const Rest &... rest) {
        using H = types::remove_cvref_t<Head>;

        if constexpr (types::is_vec_or_swizzle<H>) {
            For(range(dim_of_v<H>)) {
                this->Data[index] = T(head[it]);
                ++index;
            }
        } else {
            this->Data[index] = T(head);
            ++index;
        }
        assign_from_mixed(index, rest...);
    }

    void assign_from_mixed(s64 index) {
        // Terminator. We assign the rest of the vector to zero (this will probably never happen
        // because we require the sum of the mixed arguments to be the same as the dimension.. but in case we do something weird in the future!).
        for (; index < DIM; index++) this->Data[index] = T(0);
    }
};

template <typename SimdT, s64... Indices>
auto shuffle_reverse(SimdT arg, integer_sequence<Indices...>) {
    return SimdT::template shuffle<Indices...>(arg);
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...>::operator vec<typename swizzle<VectorDataU, Indices...>::T, sizeof...(Indices), false>() const {
    using DestVecT = vec<T, sizeof...(Indices), false>;

    if constexpr (has_simd<VectorDataU> && has_simd<DestVecT>) {
        using SourceSimdT = decltype(types::declval<VectorDataU>().Simd);
        using DestSimdT = decltype(types::declval<DestVecT>().Simd);

        constexpr s64 VectorDataDim = vec_info<VectorDataU>::DIM;
        if constexpr (types::is_same<SourceSimdT, DestSimdT>) {
            auto &sourceSimd = ((VectorDataU *) this)->Simd;
            // :SimdForVec3: We support SIMD for vectors with a dimension of 3 and we treat them as having 4 components
            if constexpr (sizeof...(Indices) == 3 && VectorDataDim == 3 && VectorDataDim == 4) {
                return {DestVecT::FROM_SIMD, shuffle_reverse(sourceSimd, typename reverse_sequence<integer_sequence<Indices..., 3>>::type{})};
            } else if constexpr (sizeof...(Indices) == 4 && VectorDataDim == 3 && VectorDataDim == 4) {
                return {DestVecT::FROM_SIMD, shuffle_reverse(sourceSimd, typename reverse_sequence<integer_sequence<Indices...>>::type{})};
            } else if constexpr (sizeof...(Indices) == 2 && VectorDataDim == 2) {
                return {DestVecT::FROM_SIMD, shuffle_reverse(sourceSimd, typename reverse_sequence<integer_sequence<Indices...>>::type{})};
            }
        }
    }
    return DestVecT(((typename vec_info<VectorDataU>::T *) this)[Indices]...);
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...>::operator vec<typename swizzle<VectorDataU, Indices...>::T, sizeof...(Indices), true>() const {
    return vec<T, sizeof...(Indices), true>(this->Data[Indices]...);
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...> &swizzle<VectorDataU, Indices...>::operator=(const vec<T, sizeof...(Indices), false> &rhs) {
    if ((typename vec_info<VectorDataU>::T *) this != rhs.Data) {
        assign<Indices...>(rhs.Data);
    } else {
        vec<T, sizeof...(Indices), false> temp = rhs;
        *this                                  = temp;
    }
    return *this;
}

template <typename VectorDataU, s64... Indices>
swizzle<VectorDataU, Indices...> &swizzle<VectorDataU, Indices...>::operator=(const vec<T, sizeof...(Indices), true> &rhs) {
    if ((typename vec_info<VectorDataU>::T *) this != rhs.Data) {
        assign<Indices...>(rhs.Data);
    } else {
        vec<T, sizeof...(Indices), false> temp = rhs;
        *this                                  = temp;
    }
    return *this;
}

template <typename T, bool Packed = false>
using vec1 = vec<T, 1, Packed>;

template <typename T, bool Packed = false>
using vec2 = vec<T, 2, Packed>;

template <typename T, bool Packed = false>
using vec3 = vec<T, 3, Packed>;

template <typename T, bool Packed = false>
using vec4 = vec<T, 4, Packed>;

template <typename T, bool Packed = false>
using vec5 = vec<T, 5, Packed>;

template <typename T, bool Packed = false>
using vec6 = vec<T, 6, Packed>;

using v1 = vec1<f32>;
using v2 = vec2<f32>;
using v3 = vec3<f32>;
using v4 = vec4<f32>;
using v5 = vec5<f32>;
using v6 = vec6<f32>;

template <s64 Dim, bool Packed = false>
using vecf = vec<f32, Dim, Packed>;

LSTD_END_NAMESPACE
