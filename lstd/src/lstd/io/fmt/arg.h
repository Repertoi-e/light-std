#pragma once

#include "../../memory/stack_array.h"
#include "parse_context.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct arg {
    type Type = type::NONE;
    value Value;

    struct handle {
        value::custom Custom;

        handle(value::custom val) : Custom(val) {}
        void format(format_context *f) { Custom.FormatFunction(Custom.Data, f); }
    };
};

namespace internal {
struct named_arg_base {
    string Name;

    // The serialized argument
    char Data[sizeof(arg)]{};

    named_arg_base(const string &name) : Name(name) {}

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
    inline static auto TypeTag = type_constant_v<value_t>;

    named_arg(const string &name, const T &value) : named_arg_base(name), Value(value) {}
};
}  // namespace internal

// Returns a named argument to be used in a formatting function.
// The named argument holds a reference and does not extend the lifetime of its arguments.
template <typename T>
auto named(const string &name, const T &arg) {
    return internal::named_arg<T>(name, arg);
}

// Disable construction of nested named arguments
template <typename T>
void named(const string &, internal::named_arg<T>) = delete;

template <typename T>
arg make_arg(const T &value);

// Maps formatting arguments to core types
struct arg_mapper {
    using long_t = type_select_t<sizeof(long) == sizeof(int), s32, s64>;
    using ulong_t = type_select_t<sizeof(long) == sizeof(int), u32, u64>;

    s32 map(char val) { return val; }
    s32 map(signed char val) { return val; }
    u64 map(unsigned char val) { return val; }
    s32 map(s16 val) { return val; }
    u32 map(u16 val) { return val; }
    s32 map(s32 val) { return val; }
    u32 map(u32 val) { return val; }
    long_t map(long val) { return val; }
    ulong_t map(unsigned long val) { return val; }
    s64 map(s64 val) { return val; }
    u64 map(u64 val) { return val; }
    bool map(bool val) { return val; }

    u32 map(wchar_t val) { return val; }
    u32 map(char32_t val) { return val; }

    f64 map(f32 val) { return (f64) val; }
    f64 map(f64 val) { return val; }
    void map(long double val) { assert(false && "Argument of type 'long double' is not supported"); }

    string map(char *val) { return string(val); }
    string map(const char *val) { return string(val); }

    template <typename T>
    enable_if_t<is_constructible_v<string, T>, string> map(const T &val) {
        return string(val);
    }

    string map(string val) { return (string) val; }
    const void *map(void *val) { return val; }
    const void *map(const void *val) { return val; }

    template <typename T>
    s32 map(const T *) {
        // Formatting of arbitrary pointers is disallowed.
        // If you want to output a pointer cast it to "void *" or "const void *".
        static_assert(is_same_v<T, void>, "Formatting of non-void pointers is disallowed");
        return 0;
    }

    template <typename T>
    enable_if_t<!is_constructible_v<string, T> && has_formatter<T>::value, const T &> map(const T &val) {
        return val;
    }

    template <typename T, typename = enable_if_t<is_enum_v<T>>>
    auto map(const T &val) {
        return (underlying_type_t<T>) (val);
    }

    template <typename T>
    const internal::named_arg_base &map(const internal::named_arg<T> &val) {
        auto result = make_arg(val.Value);
        copy_memory(const_cast<char *>(val.Data), &result, sizeof(arg));
        return val;
    }
};

namespace internal {

template <typename>
struct sfinae_true : true_t {};

template <typename T>
static auto test_formatting(int) -> sfinae_true<decltype(arg_mapper{}.map(declval<T>()))>;
template <typename>
static auto test_formatting(long) -> false_t;
}  // namespace internal

template <typename T>
struct can_be_formatted : decltype(internal::test_formatting<T>(0)) {};

template <typename T>
constexpr bool can_be_formatted_v = can_be_formatted<T>::value;

// !!!
// If you get a compiler error here it's probably because you passed in an argument that can't be formatted
// !!!
template <typename T>
constexpr auto mapped_type_constant_v = type_constant_v<decltype(arg_mapper{}.map(declval<T>()))>;

template <typename T>
arg make_arg(const T &value) {
    static_assert(can_be_formatted_v<T>, "Argument doesn't have a formatter.");
    return {mapped_type_constant_v<T>, arg_mapper().map(value)};
}

template <bool IsPacked, typename T>
enable_if_t<IsPacked, value> make_arg(const T &value) {
    static_assert(can_be_formatted_v<T>, "Argument doesn't have a formatter.");
    return arg_mapper().map(value);
}

template <bool IsPacked, typename T>
enable_if_t<!IsPacked, arg> make_arg(const T &value) {
    static_assert(can_be_formatted_v<T>, "Argument doesn't have a formatter.");
    return make_arg(value);
}

// Visits an argument dispatching to the appropriate visit method based on the argument type
template <typename Visitor>
auto visit_fmt_arg(Visitor visitor, arg ar) -> decltype(visitor(0)) {
    switch (ar.Type) {
        case type::NONE:
            break;
        case type::NAMED_ARG:
            assert(false && "Invalid argument type");
            break;
        case type::S32:
            return visitor(ar.Value.S32);
        case type::U32:
            return visitor(ar.Value.U32);
        case type::S64:
            return visitor(ar.Value.S64);
        case type::U64:
            return visitor(ar.Value.U64);
        case type::BOOL:
            return visitor(ar.Value.S32 != 0);
        case type::F64:
            return visitor(ar.Value.F64);
        case type::STRING:
            return visitor(ar.Value.String);
        case type::POINTER:
            return visitor(ar.Value.Pointer);
        case type::CUSTOM:
            return visitor(typename arg::handle(ar.Value.Custom));
    }
    return visitor(unused{});
}

namespace internal {
constexpr u64 IS_UNPACKED_BIT = 1ull << 63;
constexpr u32 MAX_PACKED_ARGS = 15;

template <typename Dummy>
u64 get_packed_fmt_types() {
    return 0ull;
}

template <typename Dummy, typename Arg, typename... Args>
u64 get_packed_fmt_types() {
    return (u64) mapped_type_constant_v<Arg> | (get_packed_fmt_types<Dummy, Args...>() << 4);
}
}  // namespace internal

template <typename... Args>
struct args_store {
    static constexpr s64 NUM_ARGS = sizeof...(Args);

    static constexpr bool IS_PACKED = NUM_ARGS < internal::MAX_PACKED_ARGS;

    // If the arguments are not packed, add one more element to mark the end.
    static constexpr s64 DATA_SIZE = NUM_ARGS + (IS_PACKED && NUM_ARGS != 0 ? 0 : 1);

    using value_t = type_select_t<IS_PACKED, value, arg>;
    stack_array<value_t, DATA_SIZE> Data;

    u64 Types = 0;

    void populate(const Args &... args) {
        Data = {make_arg<IS_PACKED>(args)...};
        Types = IS_PACKED ? internal::get_packed_fmt_types<unused, Args...>() : internal::IS_UNPACKED_BIT | NUM_ARGS;
    }
};

struct args {
    u64 Types = 0;
    union {
        const value *Values;
        const arg *Args;
    };
    s64 Count = 0;

    args() = default;

    template <typename... Args>
    args(const args_store<Args...> &store) : Types(store.Types), Count(sizeof...(Args)) {
        set_data(store.Data.Data);
    }

    void set_data(const value *values) { Values = values; }
    void set_data(const arg *ars) { Args = ars; }

    bool is_packed() { return !(Types & internal::IS_UNPACKED_BIT); }

    type get_type(s64 index) {
        u64 shift = (u64) index * 4;
        return (type)((Types & (0xfull << shift)) >> shift);
    }

    s64 max_size() { return (s64)(is_packed() ? internal::MAX_PACKED_ARGS : Types & ~internal::IS_UNPACKED_BIT); }

    // Doesn't support negative indexing
    arg get_arg(s64 index) {
        arg result;
        if (!is_packed()) {
            if (index < max_size()) result = Args[index];
            return result;
        }
        if (index > internal::MAX_PACKED_ARGS) return result;

        result.Type = get_type(index);
        if (result.Type == type::NONE) return result;

        result.Value = Values[index];
        if (result.Type == type::NAMED_ARG) {
            result = result.Value.NamedArg->deserialize();
        }
        return result;
    }
};

struct arg_map : non_copyable {
    // A map from argument names and their values (for named arguments)
    struct entry {
        string Name;
        arg Arg;
    };

    entry *Entries = null;
    u32 Size = 0;

    arg_map() = default;
    ~arg_map() { free(Entries); }

    void ensure_initted(args ars) {
        if (Entries) return;

        Entries = allocate_array(entry, ars.max_size());

        if (ars.is_packed()) {
            s64 i = 0;
            while (true) {
                auto type = ars.get_type(i);

                if (type == type::NONE) break;
                if (type == type::NAMED_ARG) {
                    add(ars.Values[i]);
                }
                ++i;
            }
        } else {
            s64 i = 0;
            while (true) {
                auto type = ars.Args[i].Type;
                if (type == type::NONE) break;
                if (type == type::NAMED_ARG) {
                    add(ars.Args[i].Value);
                }
                ++i;
            }
        }
    }

    void add(const value &value) { Entries[Size++] = {value.NamedArg->Name, value.NamedArg->deserialize()}; }

    arg find(const string &name) {
        for (auto *it = Entries, *end = Entries + Size; it != end; ++it) {
            if (it->Name == name) return it->Arg;
        }
        return arg();
    }
};

}  // namespace fmt

LSTD_END_NAMESPACE
