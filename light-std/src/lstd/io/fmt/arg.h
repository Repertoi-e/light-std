#pragma once

#include "parse.h"
#include "value.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

template <typename T>
constexpr arg make_arg(const T &value) {
    arg result;
    result.Type = type_constant_v<T>;
    result.Value = arg_mapper().map(value);
    return result;
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

template <typename Dummy>
constexpr u64 get_packed_fmt_types() {
    return 0ull;
}

// !!!
// If you get a compiler error here it's probably because you passed in an argument that can't be formatted.
// !!!
template <typename T>
using mapped_type_constant = type_constant<decltype(arg_mapper{}.map(declval<T>()))>;

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

// Constructs an _fmt::args_store_ object that contains references to arguments
// and can be implicitly converted to _fmt::arg_.
template <typename... Args>
args_store<Args...> make_fmt_args(const Args &... args) {
    return {args...};
}

template <typename... Args>
args_store<Args...> make_fmt_args_checked(string_view fmtString, const Args &... args) {
    check_format_string<Args...>(fmtString);
    return {args...};
}

struct args {
    u64 Types = 0;
    union {
        value *Values;
        arg *Args;
    };

    args() {}

    template <typename... Args>
    args(args_store<Args...> store) : Types(store.TYPES) {
        set_data(store.Data);
    }

    void set_data(value *values) { Values = values; }
    void set_data(arg *ars) { Args = ars; }

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

namespace internal {
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

    void add(value value) { Entries[Size++] = entry{value.NamedArg->Name, value.NamedArg->deserialize()}; }

    arg find(string_view name) const {
        for (auto *it = Entries, *end = Entries + Size; it != end; ++it) {
            if (it->Name == name) return it->Arg;
        }
        return {};
    }
};
}  // namespace internal

}  // namespace fmt

LSTD_END_NAMESPACE
