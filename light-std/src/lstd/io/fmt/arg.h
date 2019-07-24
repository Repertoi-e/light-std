#pragma once

#include "parse_context.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct arg {
    type Type = type::NONE;
    value Value;

    struct handle {
        value::custom Custom;

        handle(value::custom val) : Custom(val) {}
        void format(format_context *f) const { Custom.FormatFunction(Custom.Data, f); }
    };
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
}  // namespace internal

// Returns a named argument to be used in a formatting function.
// The named argument holds a reference and does not extend the lifetime of its arguments.
template <typename T>
constexpr auto named(string_view name, const T &arg) {
    return internal::named_arg<T>(name, arg);
}

// Disable construction of nested named arguments
template <typename T>
void named(string_view, internal::named_arg<T>) = delete;

template <typename T>
constexpr arg make_arg(const T &value);

// Maps formatting arguments to core types
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

    constexpr f64 map(f32 val) { return (f64) val; }
    constexpr f64 map(f64 val) { return val; }

    constexpr const byte *map(byte *val) { return val; }
    constexpr const byte *map(const byte *val) { return val; }

    template <typename T>
    constexpr enable_if_t<is_constructible_v<string_view, T>, string_view> map(const T &val) {
        return string_view(val);
    }

    string_view map(string val) { return (string_view) val; }
    constexpr const void *map(void *val) { return val; }
    constexpr const void *map(const void *val) { return val; }

    template <typename T>
    constexpr s32 map(const T *) {
        // Formatting of arbitrary pointers is disallowed.
        // If you want to output a pointer cast it to "void *" or "const void *".
        static_assert(is_same_v<T, void>, "Formatting of non-void pointers is disallowed");
        return 0;
    }

    template <typename T>
    constexpr enable_if_t<!is_constructible_v<string_view, T> && has_formatter<T>::value, const T &> map(const T &val) {
        return val;
    }

    template <typename T, typename = enable_if_t<is_enum_v<T>>>
    constexpr auto map(const T &val) {
        return (underlying_type_t<T>) (val);
    }

    template <typename T>
    constexpr const internal::named_arg_base &map(const internal::named_arg<T> &val) {
        auto result = make_arg(val.value);
        copy_memory_constexpr(val.data, &result, sizeof(arg));
        return val;
    }
};

template <typename T>
constexpr arg make_arg(const T &value) {
    return {type_constant_v<T>, arg_mapper().map(value)};
}

template <bool IsPacked, typename T>
enable_if_t<IsPacked, value> make_arg(const T &value) {
    return arg_mapper().map(value);
}

template <bool IsPacked, typename T>
enable_if_t<!IsPacked, arg> make_arg(const T &value) {
    return make_arg(value);
}

// Visits an argument dispatching to the appropriate visit method based on the argument type.
template <typename Visitor>
constexpr auto visit_fmt_arg(Visitor visitor, arg ar) -> decltype(visitor(0)) {
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
        case type::CSTRING:
            return visitor(ar.Value.ByteView.begin());
        case type::STRING:
            return visitor(string_view(ar.Value.ByteView.begin(), ar.Value.ByteView.size()));
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

// !!!
// If you get a compiler error here it's probably because you passed in an argument that can't be formatted.
// !!!
template <typename T>
using mapped_type_constant = type_constant<decltype(arg_mapper{}.map(declval<T>()))>;

template <typename Dummy>
constexpr u64 get_packed_fmt_types() {
    return 0ull;
}

template <typename Dummy, typename Arg, typename... Args>
constexpr u64 get_packed_fmt_types() {
    return (u64) mapped_type_constant<Arg>::value | (get_packed_fmt_types<Dummy, Args...>() << 4);
}
}  // namespace internal

template <typename... Args>
struct args_store {
    static constexpr size_t NUM_ARGS = sizeof...(Args);

    static const bool IS_PACKED = NUM_ARGS < internal::MAX_PACKED_ARGS;

    // If the arguments are not packed, add one more element to mark the end.
    static constexpr size_t DATA_SIZE = NUM_ARGS + (IS_PACKED && NUM_ARGS != 0 ? 0 : 1);

    using value_t = type_select_t<IS_PACKED, value, arg>;
    value_t Data[DATA_SIZE];

    static constexpr u64 TYPES =
        IS_PACKED ? internal::get_packed_fmt_types<unused, Args...>() : internal::IS_UNPACKED_BIT | NUM_ARGS;

    args_store(const Args &... args) : Data{make_arg<IS_PACKED>(args)...} {}
};

// Constructs an _args_store_ object that contains references to arguments and can be implicitly converted to _args_.
template <typename... Args>
args_store<remove_reference_t<Args>...> make_arg_store(const remove_reference_t<Args> &... args) {
    return {args...};
}

struct args {
    u64 Types = 0;
    union {
        const value *Values;
        const arg *Args;
    };
    size_t Count = 0;

    args() = default;

    template <typename... Args>
    args(const args_store<Args...> &store) : Types(store.TYPES), Count(sizeof...(Args)) {
        set_data(store.Data);
    }

    void set_data(const value *values) { Values = values; }
    void set_data(const arg *ars) { Args = ars; }

    bool is_packed() const { return (Types & internal::IS_UNPACKED_BIT) == 0; }

    type get_type(size_t index) const {
        u64 shift = index * 4;
        return (type)((Types & (0xfull << shift)) >> shift);
    }

    size_t max_size() const {
        return (size_t)(is_packed() ? internal::MAX_PACKED_ARGS : Types & ~internal::IS_UNPACKED_BIT);
    }

    arg get_arg(size_t index) const {
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
        string_view Name;
        arg Arg;
    };

    entry *Entries = null;
    u32 Size = 0;

    arg_map() = default;
    ~arg_map() {
        if (Entries) delete[] Entries;
    }

    void ensure_initted(args ars) {
        if (Entries) return;

        Entries = new entry[ars.max_size()];

        if (ars.get_type(internal::MAX_PACKED_ARGS - 1) == type::NONE) {
            u32 i = 0;
            while (true) {
                auto type = ars.get_type(i);
                if (type == type::NONE) {
                    return;
                } else if (type == type::NAMED_ARG) {
                    add(ars.Values[i]);
                }
                ++i;
            }
        }
        u32 i = 0;
        while (true) {
            auto type = ars.Args[i].Type;
            if (type == type::NONE) {
                return;
            } else if (type == type::NAMED_ARG) {
                add(ars.Args[i].Value);
            }
            ++i;
        }
    }

    void add(const value &value) { Entries[Size++] = {value.NamedArg->Name, value.NamedArg->deserialize()}; }

    arg find(string_view name) const {
        for (auto *it = Entries, *end = Entries + Size; it != end; ++it) {
            if (it->Name == name) return it->Arg;
        }
        return arg();
    }
};

}  // namespace fmt

LSTD_END_NAMESPACE
