#pragma once

#include "scalar_types.h"

// :AvoidSTDs:
// Normally initializer_list would be included but if we avoid using headers from the C++ STD we define our own implementation here.
// Note: You must tell us with a macro: LSTD_DONT_DEFINE_STD.
//
// By default we avoid STDs (like in real life) but if e.g. a library relies on it we would get definition errors.
// In general this library can work WITH or WITHOUT the normal standard library.
#if defined LSTD_DONT_DEFINE_STD
#include <initializer_list>
#include <stdarg.h>
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

// Personal preference
// I prefer to type null over nullptr but they are exactly the same
constexpr auto null = nullptr;

using null_t = decltype(nullptr);

template <typename T>
using initializer_list = std::initializer_list<T>;
