#pragma once

#include "../../internal/common.h"
#include "../../memory/string.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {
enum class type {
    NONE = 0,
    NAMED_ARG,

    // Integers
    S64,
    U64,
    BOOL,
    LAST_INTEGRAL = BOOL,

    // Floats
    F64,
    LAST_ARITHMETIC = F64,

    STRING,
    POINTER,
    CUSTOM
};

constexpr type operator|(type lhs, type rhs) {
    using T = underlying_type_t<type>;
    return (type)((T) lhs | (T) rhs);
}
constexpr type &operator|=(type &lhs, type rhs) {
    using T = underlying_type_t<type>;
    lhs = (type)((T) lhs | (T) rhs);
    return lhs;
}

constexpr bool is_fmt_type_integral(type type) {
    assert(type != type::NAMED_ARG);
    return type > type::NONE && type <= type::LAST_INTEGRAL;
}

constexpr bool is_fmt_type_arithmetic(type type) {
    assert(type != type::NAMED_ARG);
    return type > type::NONE && type <= type::LAST_ARITHMETIC;
}

template <typename T>
struct type_constant : integral_constant<type, type::CUSTOM> {};

#define TYPE_CONSTANT(Type, constant) \
    template <>                       \
    struct type_constant<Type> : integral_constant<type, constant> {}

namespace internal {
struct named_arg_base;
}

TYPE_CONSTANT(const internal::named_arg_base &, type::NAMED_ARG);
TYPE_CONSTANT(char, type::S64);
TYPE_CONSTANT(s32, type::S64);
TYPE_CONSTANT(s64, type::S64);
TYPE_CONSTANT(u32, type::U64);
TYPE_CONSTANT(u64, type::U64);
TYPE_CONSTANT(bool, type::BOOL);
TYPE_CONSTANT(f64, type::F64);
TYPE_CONSTANT(string, type::STRING);
TYPE_CONSTANT(const void *, type::POINTER);
#undef TYPE_CONSTANT

template <typename T>
constexpr auto type_constant_v = type_constant<T>::value;

//
// Specialize this for custom types
//
// template <>
// struct formatter<my_type> {
//     formatter() = default;
//
// 	void format(my_type val, format_context *f) {
// 		...
// 	}
// };
template <typename T, typename Enable = void>
struct formatter {
    formatter() = delete;
};

template <typename T>
using has_formatter = is_constructible<formatter<T>>;

template <typename T>
constexpr bool has_formatter_v = has_formatter<T>::value;

struct format_context;

// Contains a value of any type
struct value {
    struct custom {
        const void *Data;
        void (*FormatFunction)(const void *arg, format_context *f);
    };

    union {
        s64 S64;
        u64 U64;
        f64 F64;

        const void *Pointer;
        string String;

        const internal::named_arg_base *NamedArg;

        custom Custom;
    };

    value(s64 value = 0) : S64(value) {}
    value(u64 value) : U64(value) {}
    value(int value) : S64(value) {}
    value(f64 value) : F64(value) {}
    value(const void *value) : Pointer(value) {}
    value(const string &value) : String(value) {}
    value(const internal::named_arg_base &value) : NamedArg(&value) {}

    template <typename T>
    value(const T *value) {
        Custom.Data = value;
        Custom.FormatFunction = format_custom_arg<T>;
    }

   private:
    template <typename T>
    static void format_custom_arg(const void *arg, format_context *f) {
        formatter<remove_cvref_t<T>> formatter;
        formatter.format(*(const T *) (arg), f);
    }
};
}  // namespace fmt

LSTD_END_NAMESPACE
