#pragma once

#include "../../common.h"
#include "../../intrin.h"

#include "../../storage/string.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {
enum class type {
    NONE = 0,
    NAMED_ARG,

    // Integers
    S32,
    U32,
    S64,
    U64,
    BOOL,
    LAST_INTEGER = BOOL,

    // Floats
    F64,
    LAST_NUMERIC = F64,

    CSTRING,
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
    return type > type::NONE && type <= type::LAST_INTEGER;
}

constexpr bool is_fmt_type_numeric(type type) {
    assert(type != type::NAMED_ARG);
    return type > type::NONE && type <= type::LAST_NUMERIC;
}

// Specialize this for custom types that may not be POD or have data that isn't serialized, e.g. pointers
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

struct format_context;

struct custom_value {
    const void *Data;
    void (*FormatFunction)(const void *arg, format_context *f);
};

template <typename T>
struct type_constant : integral_constant<type, type::CUSTOM> {};

#define TYPE_CONSTANT(Type, constant) \
    template <>                       \
    struct type_constant<Type> : integral_constant<type, constant> {}

namespace internal {
struct named_arg_base;
}

TYPE_CONSTANT(const internal::named_arg_base &, type::NAMED_ARG);
TYPE_CONSTANT(s32, type::S32);
TYPE_CONSTANT(u32, type::U32);
TYPE_CONSTANT(s64, type::S64);
TYPE_CONSTANT(u64, type::U64);
TYPE_CONSTANT(bool, type::BOOL);
TYPE_CONSTANT(byte, type::S32);
TYPE_CONSTANT(f32, type::F64);
TYPE_CONSTANT(f64, type::F64);
TYPE_CONSTANT(const byte *, type::STRING);
TYPE_CONSTANT(string_view, type::STRING);
TYPE_CONSTANT(const void *, type::POINTER);
#undef TYPE_CONSTANT

template <typename T>
constexpr auto type_constant_v = type_constant<T>::value;

struct value {
    union {
        s32 S32;
        u32 U32;
        s64 S64;
        u64 U64;
        f64 F64;
        const void *Pointer;
        array_view<byte> ByteView;
        custom_value Custom;
        const internal::named_arg_base *NamedArg;
    };

    constexpr value(s32 value = 0) : S32(value) {}
    value(u32 value) : U32(value) {}
    value(s64 value) : S64(value) {}
    value(u64 value) : U64(value) {}
    value(f64 value) : F64(value) {}
    value(bool value) : S32(value) {}
    value(const void *value) : Pointer(value) {}
    value(const byte *value) : ByteView(value, value + cstring_strlen(value)) {}
    value(string value) : ByteView(value.Data, value.Data + value.ByteLength) {}
    value(string_view value) : ByteView(value.Data, value.Data + value.ByteLength) {}
    value(const internal::named_arg_base &value) : NamedArg(&value) {}

    template <typename T>
    value(const T &value) {
        Custom.Data = &value;
        Custom.FormatFunction = format_custom_arg<T>;
    }

   private:
    template <typename T>
    static void format_custom_arg(const void *arg, format_context *f) {
        formatter<T> f;
        f.format(*(const T *) (arg), f);
    }
};  // namespace fmt

struct arg {
    value Value;
    type Type = type::NONE;

    struct handle {
        custom_value Custom;

        handle(custom_value val) : Custom(val) {}
        void format(format_context *f) const { Custom.FormatFunction(Custom.Data, f); }
    };

    explicit operator bool() const { return Type != type::NONE; }
};

namespace internal {
struct named_arg_base {
    string_view Name;

    // The serialized argument
    byte Data[sizeof(arg)];

    named_arg_base(string_view name) : Name(name) {}

    arg deserialize() const {
        arg result;
        copy_memory(&result, Data, sizeof(arg));
        return result;
    }
};

template <typename T>
struct named_arg : named_arg_base {
    const T &Value;

    using value_t = T;
    static constexpr auto TypeTag = type_constant_v<value_t>;

    named_arg(string_view name, const T &value) : named_arg_base(name), Value(value) {}
};

// @TODO Disable construction of nested arguments
}  // namespace internal

template <typename T>
constexpr arg make_arg(const T &value);

// Maps formatting arguments to core types.
struct arg_mapper {
    using long_t = type_select_t<sizeof(long) == sizeof(int), s32, s64>;
    using ulong_t = type_select_t<sizeof(long) == sizeof(int), u32, u64>;

    constexpr s32 map(signed char val) { return val; }
    constexpr u64 map(unsigned char val) { return val; }
    constexpr s32 map(s16 val) { return val; }
    constexpr u32 map(u16 val) { return val; }
    constexpr s32 map(s32 val) { return val; }
    constexpr u32 map(u32 val) { return val; }
    constexpr long_t map(long val) { return val; }
    constexpr ulong_t map(unsigned long val) { return val; }
    constexpr s64 map(s64 val) { return val; }
    constexpr u64 map(u64 val) { return val; }
    constexpr bool map(bool val) { return val; }

    template <typename T, enable_if_t<is_same_v<T, byte>> = 0>
    constexpr byte map(const T &val) {
        return val;
    }

    constexpr f64 map(f32 val) { return (f64) val; }
    constexpr f64 map(f64 val) { return val; }

    constexpr const byte *map(byte *val) { return val; }
    constexpr const byte *map(const byte *val) { return val; }

    template <typename T, enable_if_t<is_constructible_v<string_view, T>, int> = 0>
    constexpr string_view map(const T &val) {
        return string_view(val);
    }

    inline string_view make_value(string val) { return (string_view) val; }
    constexpr const void *map(void *val) { return val; }
    constexpr const void *map(const void *val) { return val; }

    template <typename T>
    constexpr s32 map(const T *) {
        // Formatting of arbitrary pointers is disallowed.
        // If you want to output a pointer cast it to "void *" or "const void *".
        static_assert(is_same_v<T, void>, "Formatting of non-void pointers is disallowed");
        return 0;
    }

    template <typename T, enable_if_t<is_enum_v<T> && !has_formatter<T>::value> = 0>
    constexpr s32 map(const T &val) {
        return (s32)(val);
    }

    template <typename T,
              enable_if_t<!is_constructible_v<string_view, T> && !is_same_v<T, byte> && has_formatter<T>::value> = 0>
    constexpr const T &map(const T &val) {
        return val;
    }

    template <typename T>
    constexpr const internal::named_arg_base &map(const internal::named_arg<T> &val) {
        auto result = make_arg(val.value);
        copy_memory_constexpr(val.data, &result, sizeof(arg));
        return val;
    }
};
}  // namespace fmt

LSTD_END_NAMESPACE

// :ExplicitDeclareIsPod
DECLARE_IS_POD(fmt::value, true)
DECLARE_IS_POD(fmt::arg, true)