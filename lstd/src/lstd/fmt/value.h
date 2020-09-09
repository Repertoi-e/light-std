#pragma once

#include "../memory/string.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {
enum class type {
    NONE = 0,

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

constexpr bool is_fmt_type_integral(type type) {
    return type > type::NONE && type <= type::LAST_INTEGRAL;
}

constexpr bool is_fmt_type_arithmetic(type type) {
    return type > type::NONE && type <= type::LAST_ARITHMETIC;
}

struct named_arg;

namespace internal {
template <typename T>
struct type_constant : types::integral_constant<type, type::CUSTOM> {};

#define TYPE_CONSTANT(Type, constant) \
    template <>                       \
    struct type_constant<Type> : types::integral_constant<type, constant> {}

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
}  // namespace internal

template <typename T>
constexpr auto type_constant_v = internal::type_constant<types::remove_cvref_t<T>>::value;

//
// Specialize this for custom types
//
// template <>
// struct formatter<my_type> {
//     formatter() {}
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
concept has_formatter = types::is_constructible<formatter<T>>;

// Can T be formatted with a custom formatter
// template <typename T>
// concept formattable = requires(T, t) { {formatter<T>{}}; };

struct format_context;

// Contains a value of any type
struct value {
    struct custom {
        const void *Data;
        void (*FormatFunction)(const void *arg, format_context *f);

        void format(format_context *f) const { FormatFunction(Data, f); }
    };

    union {
        s64 S64;
        u64 U64;
        f64 F64;

        const void *Pointer;
        string String;

        custom Custom;
    };

    value(s64 value = 0) : S64(value) {}
    value(u64 value) : U64(value) {}
    value(int value) : S64(value) {}
    value(f64 value) : F64(value) {}
    value(const void *value) : Pointer(value) {}
    value(const string &value) : String(value) {}

    template <typename T>
    value(const T *value) {
        Custom.Data = value;
        Custom.FormatFunction = format_custom_arg<T>;
    }

   private:
    template <typename T>
    static void format_custom_arg(const void *arg, format_context *f) {
        formatter<types::remove_cvref_t<T>> formatter;
        formatter.format(*(const T *) (arg), f);
    }
};
}  // namespace fmt

LSTD_END_NAMESPACE
