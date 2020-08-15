#pragma once

#include "../../memory/stack_array.h"
#include "../../memory/string.h"
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

// Maps formatting arguments to types that can be used to construct a fmt::value.
template <typename U>
auto map_arg(const U &val) {
    using T = typename remove_cv_t<remove_reference_t<U>>;

    static_assert(!is_same_v<T, long double>, "Argument of type 'long double' is not supported");

    if constexpr (is_arithmetic_v<T> || is_same_v<T, string::code_point_ref>) {
        if constexpr (is_same_v<bool, T>) {
            return (bool) val;
        } else if constexpr (is_floating_point_v<T>) {
            return (f64) val;
        } else if constexpr (is_signed_v<T>) {
            return (s64) val;
        } else {
            return (u64) val;
        }
    }
    else if constexpr (is_enum_v<T>) {
        return map_arg((underlying_type_t<T>) val);
    }
    else if constexpr (is_same_v<string, T> || is_constructible_v<string, T>) {
        return string(val);
    }
    else if constexpr (is_pointer_v<T>) {
        static_assert(is_same_v<T, void *>, "Formatting of non-void pointers is disallowed");
        return (const void *) val;
    }
    else if constexpr (has_formatter_v<T>) {
        return &val;
    }
    else {
        static_assert(false, "Argument doesn't have a formatter")
    }
}

template <typename T>
const internal::named_arg_base &map_arg(const internal::named_arg<T> &val) {
    auto result = make_arg(val.Value);
    copy_memory(const_cast<char *>(val.Data), &result, sizeof(arg));
    return val;
}

// !!!
// If you get a compiler error here it's probably because you passed in an argument that can't be formatted
// To format custom types, implement a fmt::formatter specialization.
// !!!
template <typename T>
constexpr auto mapped_type_constant_v = type_constant_v<decltype(map_arg(declval<T>()))>;

template <typename T>
arg make_arg(const T &v) { return {mapped_type_constant_v<T>, map_arg(v)}; }

template <bool IsPacked, typename T>
enable_if_t<IsPacked, value> make_arg(const T &v) {
    const auto &mapped = map_arg(v);
    value result = value(mapped);
    return result;
}

template <bool IsPacked, typename T>
enable_if_t<!IsPacked, arg> make_arg(const T &v) { return arg(make_arg(v)); }

// Visits an argument dispatching to the appropriate visit method based on the argument type
template <typename Visitor>
auto visit_fmt_arg(Visitor &&visitor, arg ar) -> decltype(visitor(0)) {
    switch (ar.Type) {
        case type::NONE:
            break;
        case type::NAMED_ARG:
            assert(false && "Invalid argument type");
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
            return visitor(typename arg::handle(ar.Value.Custom));
    }
    return visitor(unused{});
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

// Either an array of values or arguments (just values if number is less than MAX_PACKED_ARGS)
template <typename... Args>
struct args_stack_array {
    static constexpr s64 NUM_ARGS = sizeof...(Args);

    static constexpr bool IS_PACKED = NUM_ARGS < MAX_PACKED_ARGS;

    // If the arguments are not packed, add one more element to mark the end.
    // static constexpr s64 DATA_SIZE = NUM_ARGS + (IS_PACKED && NUM_ARGS != 0 ? 0 : 1);

    using data_t = type_select_t<IS_PACKED, value, arg>;
    stack_array<data_t, NUM_ARGS> Data;

    u64 Types = 0;

    void populate(const Args &... args) {
        Data = {make_arg<IS_PACKED>(args)...};
        Types = IS_PACKED ? internal::get_packed_fmt_types<unused, Args...>() : IS_UNPACKED_BIT | NUM_ARGS;
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
    args(const args_stack_array<Args...> &store) : Types(store.Types), Count(sizeof...(Args)) {
        set_data(store.Data.Data);
    }

    void set_data(const value *values) { Values = values; }
    void set_data(const arg *ars) { Args = ars; }

    bool is_packed() { return !(Types & IS_UNPACKED_BIT); }

    type get_type(s64 index) {
        u64 shift = (u64) index * 4;
        return (type)((Types & (0xfull << shift)) >> shift);
    }

    s64 max_size() { return (s64)(is_packed() ? MAX_PACKED_ARGS : Types & ~IS_UNPACKED_BIT); }

    // Doesn't support negative indexing
    arg get_arg(s64 index) {
        if (index >= Count) return {};

        if (is_packed()) {
            if (index > MAX_PACKED_ARGS) return {};

            auto type = get_type(index);
            if (type == type::NONE) return {};

            arg result;
            result.Type = type;
            if (result.Type == type::NAMED_ARG) {
                result = Values[index].NamedArg->deserialize();
            } else {
                result.Value = Values[index];
            }
            return result;
        }
        return Args[index];
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
