#pragma once

#include "scalar_types.h"

// This file defines: initializer_list, null, null_t, tuple

// :AvoidSTDs:
// Normally initializer_list would be included but if we avoid using headers from the C++ STD we define our own implementation here.
// Note: You must tell us with a macro: LSTD_DONT_DEFINE_STD.
//
// By default we avoid STDs (like in real life) but if e.g. a library relies on it we would get definition errors.
// In general this library can work WITH or WITHOUT the normal standard library.
#if defined LSTD_DONT_DEFINE_STD
#include <stdarg.h>

#include <initializer_list>
#else
// Note: If you get many compile errors (but you have defined LSTD_DONT_DEFINE_STD).
// You probably need to define it globally, because not all headers from this library see the macro.

namespace std {
template <typename T>
struct initializer_list {
    const T *First = null;
    const T *Last = null;

    using value_type = T;
    using reference = const T &;
    using const_reference = const T &;
    using size_type = size_t;

    constexpr initializer_list() noexcept {}
    constexpr initializer_list(const T *first, const T *last) noexcept : First(first), Last(last) {}

    using iterator = const T *;
    using const_iterator = const T *;

    constexpr const T *begin() const noexcept { return First; }
    constexpr const T *end() const noexcept { return Last; }

    constexpr size_t size() const noexcept { return static_cast<size_t>(Last - First); }
};
}  // namespace std

#define va_start __crt_va_start
#define va_arg __crt_va_arg
#define va_end __crt_va_end
#define va_copy(destination, source) ((destination) = (source))
#endif

#define offset_of(s, field) ((u64) & ((s *) (0))->field)

// Personal preference
// I prefer to type null over nullptr but they are exactly the same
constexpr auto null = nullptr;

using null_t = decltype(nullptr);

LSTD_BEGIN_NAMESPACE

template <typename T>
using initializer_list = std::initializer_list<T>;

//
// We provide a bare-minimal tuple type that does the work it needs to do.
// Doesn't support types which don't have a default constructor.
//

template <typename F, typename... Rest>
struct tuple : public tuple<Rest...> {
    F First;

    constexpr tuple() {}
    constexpr tuple(const F &first, Rest &&...rest) : tuple<Rest...>(((Rest &&) rest)...), First(first) {}
};

template <typename F>
struct tuple<F> {
    F First;

    constexpr tuple() {}
    constexpr tuple(const F &first) : First(first) {}
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
    using type = tuple_element<Index - 1, tuple<Rest...>>::type;
};

template <typename F, typename... Rest>
struct tuple_element<0, tuple<F, Rest...>> {
    using type = decltype(tuple<F, Rest...>().First);
};

template <s64 Index, typename T>  // This shouldn't compile on types that are not tuples
using tuple_get_t = tuple_element<Index, T>::type;

LSTD_END_NAMESPACE

//
// We provide the following specializations for our tuple type: std::tuple_element, std::tuple_size, std::get.
// Those are needed in order to support C++17 style structured bindings.
// e.g.
//    auto [p, q] = tuple(1, 2);
//

namespace std {

#if not defined LSTD_DONT_DEFINE_STD
template <size_t I, typename T>
struct tuple_element;

template <typename T>
struct tuple_size;
#endif

template <size_t Index, typename F, typename... Rest>
struct tuple_element<Index, LSTD_NAMESPACE::tuple<F, Rest...>> {
    using type = tuple_element<Index - 1, LSTD_NAMESPACE::tuple<Rest...>>::type;
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
