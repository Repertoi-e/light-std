#pragma once

#include "cpp/arg.h"
#include "cpp/initializer_list.h"
#include "cpp/source_location.h"
#include "scalar_types.h"

//
// This file defines:
//   macros for C-style varargs,
//   initializer_list,
//   source_location,
//
//   null,
//   tuple,
//   range,
//

// Personal preference
// I prefer to type null over nullptr but they are exactly the same
constexpr auto null = nullptr;
using null_t        = decltype(nullptr);

LSTD_BEGIN_NAMESPACE

template <typename T>
using initializer_list = std::initializer_list<T>;

//
// We provide a bare-minimal tuple type that does the work it needs to do.
// Doesn't support types which don't have a default constructor.
//

template <typename F, typename... Rest>
struct tuple : tuple<Rest...> {
    F First;

    constexpr tuple() {
    }

    constexpr tuple(const F &first, Rest &&...rest)
        : tuple<Rest...>((Rest &&) rest...),
          First(first) {
    }
};

template <typename F>
struct tuple<F> {
    F First;

    constexpr tuple() {
    }

    constexpr tuple(const F &first)
        : First(first) {
    }
};

template <s64 Index, typename F, typename... Rest>
struct tuple_get_impl {
    static_assert(Index >= 0);

    // I hate C++
    constexpr static const auto &value(const tuple<F, Rest...> &t) { return tuple_get_impl<Index - 1, Rest...>::value(t); }
    constexpr static const auto &value(const tuple<F, Rest...> &&t) { return tuple_get_impl<Index - 1, Rest...>::value(t); }
    constexpr static auto &value(tuple<F, Rest...> &t) { return tuple_get_impl<Index - 1, Rest...>::value(t); }
    constexpr static auto &value(tuple<F, Rest...> &&t) { return tuple_get_impl<Index - 1, Rest...>::value(t); }
};

template <typename F, typename... Rest>
struct tuple_get_impl<0, F, Rest...> {
    // I hate C++
    constexpr static const auto &value(const tuple<F, Rest...> &t) { return t.First; }
    constexpr static const auto &value(const tuple<F, Rest...> &&t) { return t.First; }
    constexpr static auto &value(tuple<F, Rest...> &t) { return t.First; }
    constexpr static auto &value(tuple<F, Rest...> &&t) { return t.First; }
};

// I hate C++
template <s64 Index, typename... Args>
constexpr const auto &tuple_get(const tuple<Args...> &t) { return tuple_get_impl<Index, Args...>::value(t); }

template <s64 Index, typename... Args>
constexpr const auto &&tuple_get(const tuple<Args...> &&t) { return tuple_get_impl<Index, Args...>::value(t); }

template <s64 Index, typename... Args>
constexpr auto &tuple_get(tuple<Args...> &t) { return tuple_get_impl<Index, Args...>::value(t); }

template <s64 Index, typename... Args>
constexpr auto &&tuple_get(tuple<Args...> &&t) { return tuple_get_impl<Index, Args...>::value(t); }

template <size_t I, typename T>
struct tuple_element;

template <size_t Index, typename F, typename... Rest>
struct tuple_element<Index, tuple<F, Rest...>> {
    using type = typename tuple_element<Index - 1, tuple<Rest...>>::type;
};

template <typename F, typename... Rest>
struct tuple_element<0, tuple<F, Rest...>> {
    using type = decltype(tuple<F, Rest...>().First);
};

template <s64 Index, typename T>  // This shouldn't compile on types that are not tuples
using tuple_get_t = typename tuple_element<Index, T>::type;

// Python-like range functionality
// e.g.
//
//  for (auto it : range(20))        // [0, 20)
//  for (auto it : range(3, 10, 2))  // every second integer (step 2) in [3, 10)
//  for (auto it : range(10, 0, -1)) // reverse [10, 0)
//
// .. or with our For macro:
//
//  For(range(12)) {}
//
//    which is equivalent to:
//
//  For_as(it, range(12)) {}
//
struct range {
    struct iterator {
        s64 I, Step;

        constexpr iterator(s64 i, s64 step = 1) : I(i), Step(step) {}

        operator s32() const { return (s32) I; }
        operator s64() const { return I; }

        constexpr s64 operator*() const { return I; }
        constexpr iterator operator++() { return I += Step, *this; }

        constexpr iterator operator++(s32) {
            iterator temp(*this);
            return I += Step, temp;
        }

        constexpr bool operator==(iterator other) const { return Step < 0 ? (I <= other.I) : (I >= other.I); }
        constexpr bool operator!=(iterator other) const { return Step < 0 ? (I > other.I) : (I < other.I); }
    };

    iterator Begin;
    iterator End;

    constexpr range(s64 start, s64 stop, s64 step) : Begin(start, step), End(stop) {}
    constexpr range(s64 start, s64 stop) : range(start, stop, 1) {}
    constexpr range(u64 stop) : range(0, stop, 1) {}

    // Checks if a value is inside the given range.
    // This also accounts for stepping.
    constexpr bool has(s64 value) const {
        if (Begin.Step > 0 ? (value >= Begin.I && value < End.I) : (value > End.I && value <= Begin.I)) {
            s64 diff = value - Begin.I;
            if (diff % Begin.Step == 0) {
                return true;
            }
        }
        return false;
    }

    constexpr iterator begin() const { return Begin; }
    constexpr iterator end() const { return End; }
};

LSTD_END_NAMESPACE

//
// We provide the following specializations for our tuple type: std::tuple_element, std::tuple_size, std::get.
// Those are needed in order to support C++17 style structured bindings.
// e.g.
//    auto [p, q] = tuple(1, 2);
//

namespace std {

#if !defined LSTD_DONT_DEFINE_STD
template <size_t I, typename T>
struct tuple_element;

template <typename T>
struct tuple_size;
#endif

template <size_t Index, typename F, typename... Rest>
struct tuple_element<Index, LSTD_NAMESPACE::tuple<F, Rest...>> {
    using type = typename tuple_element<Index - 1, LSTD_NAMESPACE::tuple<Rest...>>::type;
};

template <typename F, typename... Rest>
struct tuple_element<0, LSTD_NAMESPACE::tuple<F, Rest...>> {
    using type = decltype(LSTD_NAMESPACE::tuple<F, Rest...>().First);
};

template <typename... Args>
struct tuple_size<LSTD_NAMESPACE::tuple<Args...>> {
    static constexpr size_t value = sizeof...(Args);
};

// I hate C++
template <s64 Index, typename... Args>
constexpr const auto &get(const LSTD_NAMESPACE::tuple<Args...> &t) { return LSTD_NAMESPACE::tuple_get<Index>(t); }

template <s64 Index, typename... Args>
constexpr const auto &get(const LSTD_NAMESPACE::tuple<Args...> &&t) { return LSTD_NAMESPACE::tuple_get<Index>(t); }

template <s64 Index, typename... Args>
constexpr auto &get(LSTD_NAMESPACE::tuple<Args...> &t) { return LSTD_NAMESPACE::tuple_get<Index>(t); }

template <s64 Index, typename... Args>
constexpr auto &get(LSTD_NAMESPACE::tuple<Args...> &&t) { return LSTD_NAMESPACE::tuple_get<Index>(t); }
}  // namespace std
