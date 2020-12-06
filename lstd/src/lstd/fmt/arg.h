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
//   * is the type a bool? maps to bool
//   * is the type an (un)integral? maps to u64 or s64
//   * is the type a floating point? maps to f64
//   * is the type a code_point_ref? maps to u64 (we want the value in that case)
//   * is the type an enum? calls map_arg again with the underlying type
// Otherwise we static_assert that the argument can't be formatted
template <typename U>
auto map_arg(const U &val) {
    using T = typename types::remove_cvref_t<U>;

    static_assert(!types::is_same<T, long double>, "Argument of type 'long double' is not supported");

    if constexpr (has_formatter<T>) {
        return &val;
    } else if constexpr (types::is_same<string, T> || types::is_constructible<string, T>) {
        return string(val);
    } else if constexpr (types::is_pointer<T>) {
        static_assert(types::is_same<T, void *>, "Formatting of non-void pointers is disallowed");
        return (const void *) val;
    } else if constexpr (types::is_same<bool, T>) {
        return (bool) val;
    } else if constexpr (types::is_signed_integral<T>) {
        return (s64) val;
    } else if constexpr (types::is_unsigned_integral<T>) {
        return (u64) val;
    } else if constexpr (types::is_floating_point<T>) {
        return (f64) val;
    } else if constexpr (types::is_same<T, string::code_point_ref>) {
        return (u64) val;
    } else if constexpr (types::is_enum<T>) {
        return map_arg((types::underlying_type_t<T>) val);
    } else {
        static_assert(false, "Argument doesn't have a formatter");
    }
}

// !!!
// If you get a compiler error here it's probably because you passed in an argument that can't be formatted
// To format custom types, implement a fmt::formatter specialization.
// !!!
template <typename T>
constexpr auto mapped_type_constant_v = type_constant_v<decltype(map_arg(types::declval<T>()))>;

template <typename T>
arg make_arg(const T &v) { return {mapped_type_constant_v<T>, map_arg(v)}; }

template <bool IsPacked, typename T>
auto make_arg(const T &v) {
    using typoed = decltype(map_arg(types::declval<T>()));
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
    return visitor(types::unused{});
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

    using T = types::select_t<IS_PACKED, value, arg>;
    stack_array<T, NUM_ARGS> Data;

    u64 Types;

    args_on_the_stack(Args &&... args) : Types(IS_PACKED ? internal::get_packed_fmt_types<types::unused, Args...>() : IS_UNPACKED_BIT | NUM_ARGS) {
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
};
}  // namespace fmt

LSTD_END_NAMESPACE
