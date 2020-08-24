#pragma once

#include "../memory/stack_array.h"
#include "parse_context.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

// Holds a type and a value. If the value is not arithmetic then the life time of the parameter isn't extended (we just hold a pointer)!
struct arg {
    type Type = type::NONE;
    value Value;
};

// Maps formatting arguments to types that can be used to construct a fmt::value.
//
// The order in which we look:
//   * does the type have a formatter? maps to &val (value then setups a function call to formatter<T>::format())
//   * is string constructible from T? then we map to string(T)
//   * is the type a pointer? if it's non-void we throw an error, otherwise we map to (void *) val
//   * is the type arithmetic or an enum? maps to bool, f64, u64 or s64
// Otherwise we static_assert that the argument can't be formatted
template <typename U>
auto map_arg(const U &val) {
    using T = typename ::type::remove_cv_t<::type::remove_reference_t<U>>;

    static_assert(!::type::is_same_v<T, long double>, "Argument of type 'long double' is not supported");

    if constexpr (has_formatter_v<T>) {
        return &val;
    } else if constexpr (::type::is_same_v<string, T> || ::type::is_constructible_v<string, T>) {
        return string(val);
    } else if constexpr (::type::is_pointer_v<T>) {
        static_assert(::type::is_same_v<T, void *>, "Formatting of non-void pointers is disallowed");
        return (const void *) val;
    } else if constexpr (::type::is_arithmetic_v<T> || ::type::is_same_v<T, string::code_point_ref>) {
        if constexpr (::type::is_same_v<bool, T>) {
            return (bool) val;
        } else if constexpr (::type::is_floating_point_v<T>) {
            return (f64) val;
        } else if constexpr (::type::is_signed_v<T>) {
            return (s64) val;
        } else {
            return (u64) val;
        }
    } else if constexpr (::type::is_enum_v<T>) {
        return map_arg((::type::underlying_type_t<T>) val);
    } else {
        static_assert(false, "Argument doesn't have a formatter")
    }
}

// Maps formatting arguments to types that can be used to construct a fmt::value
//template <typename U>
//auto map_arg(const U &val) {
//    static_assert(!::type::is_same_v<T, long double>, "Argument of type 'long double' is not supported");
//    static_assert(false, "Argument doesn't have a formatter");
//}
//
// template <typename T>
// requires(::type::is_same_v<string, ::type::remove_cv_t<::type::remove_reference_t<T>>> || ::type::is_constructible_v<string, T>) string map_arg(T val) {
//     return (string) val;
// }
//
// template <typename T>
// requires(::type::is_pointer_t<T>) s64 map_arg(T val) {
//     static_assert(::type::is_same_v<T, void *>, "Formatting of non-void pointers is disallowed");
//     return (s64) val;
// }
//
// template <typename T>
// s64 map_arg(const string::code_point_ref &val) { return (s64) val; }
//
// template <std::signed_integral T>
// s64 map_arg(T val) { return (s64) val; }
//
// template <std::unsigned_integral T>
// s64 map_arg(T val) { return (u64) val; }
//
// template <typename T>
// requires(::type::is_enum_v<T>) s64 map_arg(T val) { return map_arg(::type::underlying_type_t<T>(val)); }
//
// template <std::floating_point T>
// s64 map_arg(T val) { return (f64) val; }
//
// template <typename T>
// requires requires { {formatter<T>{}}; }
// T &map_arg(const T &val) {
//     static_assert(false);
//     return &val;
// }
//
// template <typename T>
// requires(type::is_same_v<T, type::remove_cv_t<type::remove_reference_t<T>>>) bool map_arg(T val) { return val; }

// !!!
// If you get a compiler error here it's probably because you passed in an argument that can't be formatted
// To format custom types, implement a fmt::formatter specialization.
// !!!
template <typename T>
constexpr auto mapped_type_constant_v = type_constant_v<decltype(map_arg(::type::declval<T>()))>;

template <typename T>
arg make_arg(const T &v) { return {mapped_type_constant_v<T>, map_arg(v)}; }

template <bool IsPacked, typename T>
auto make_arg(const T &v) {
    using typoed = decltype(map_arg(::type::declval<T>()));
    if constexpr (IsPacked) {
        return value(map_arg(v));
    } else {
        return make_arg(v);
    }
}

// Visits an argument dispatching with the right value based on the argument type
template <typename Visitor>
auto visit_fmt_arg(Visitor &&visitor, const arg &ar) -> decltype(visitor(0)) {
    switch (ar.Type) {
        case type::NONE:
            break;
        case type::S64:
            return visitor(ar.Value.S64);
        case type::U64:
            return visitor(ar.Value.U64);
        case type::BOOL:
            return visitor(ar.Value.S64 != 0);
        case type::F64:
            return visitor(ar.Value.F64);
        case type::STRING:
            return visitor(ar.Value.String);
        case type::POINTER:
            return visitor(ar.Value.Pointer);
        case type::CUSTOM:
            return visitor(ar.Value.Custom);
    }
    return visitor(::type::unused{});
}

namespace internal {
// Hacky template because C++
template <typename Dummy>
u64 get_packed_fmt_types() {
    return 0ull;
}

// Hacky template because C++
template <typename Dummy, typename Arg, typename... Args>
u64 get_packed_fmt_types() {
    return (u64) mapped_type_constant_v<Arg> | (get_packed_fmt_types<Dummy, Args...>() << 4);
}
}  // namespace internal

static constexpr u64 IS_UNPACKED_BIT = 1ull << 63;
static constexpr u32 MAX_PACKED_ARGS = 15;

// We can't really combine this with _args_, ugh!
// Stores either an array of values or arguments on the stack (just values if number is less than MAX_PACKED_ARGS)
template <typename... Args>
struct args_on_the_stack {
    static constexpr s64 NUM_ARGS = sizeof...(Args);
    static constexpr bool IS_PACKED = NUM_ARGS < MAX_PACKED_ARGS;

    using data_t = ::type::select_t<IS_PACKED, value, arg>;
    stack_array<data_t, NUM_ARGS> Data;

    u64 Types;

    args_on_the_stack(Args &&... args) : Types(IS_PACKED ? internal::get_packed_fmt_types<::type::unused, Args...>() : IS_UNPACKED_BIT | NUM_ARGS) {
        Data = {make_arg<IS_PACKED>(args)...};
    }
};

struct args {
    void *Data;  // (value *) or (arg *) if not packed
    s64 Count = 0;
    u64 Types = 0;

    args() {}

    template <typename... Args>
    args(const args_on_the_stack<Args...> &store) : Data((void *) store.Data.Data), Types(store.Types), Count(sizeof...(Args)) {}

    bool is_packed() { return !(Types & IS_UNPACKED_BIT); }

    type get_type(s64 index) {
        u64 shift = (u64) index * 4;
        return (type)((Types & (0xfull << shift)) >> shift);
    }

    // Doesn't support negative indexing
    arg get_arg(s64 index) {
        if (index >= Count) return {};

        if (is_packed()) {
            if (index > MAX_PACKED_ARGS) return {};

            auto type = get_type(index);
            if (type == type::NONE) return {};

            arg result;
            result.Type = type;
            result.Value = ((value *) Data)[index];
            return result;
        }
        return ((arg *) Data)[index];
    }
};
}  // namespace fmt

LSTD_END_NAMESPACE
