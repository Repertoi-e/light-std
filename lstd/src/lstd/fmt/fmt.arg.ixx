module;

#include "../io.h"

export module fmt.arg;

export {
    enum class fmt_type {
        None = 0,

        S64,
        U64,
        Bool,
        Last_Integral = Bool,

        F64,
        Last_Arithmetic = F64,

        String,
        Pointer,

        Custom
    };

    constexpr bool fmt_is_type_integral(fmt_type type) {
        return type > fmt_type::None && type <= fmt_type::Last_Integral;
    }

    constexpr bool fmt_is_type_arithmetic(fmt_type type) {
        return type > fmt_type::None && type <= fmt_type::Last_Arithmetic;
    }

    constexpr u64 FMT_IS_UNPACKED_BIT = 1ull << 63;
    constexpr u32 FMT_MAX_PACKED_ARGS = 15;

    namespace internal {
    template <typename T>
    struct type_constant : types::integral_constant<fmt_type, fmt_type::Custom> {};

#define TYPE_CONSTANT(Type, constant) \
    template <>                       \
    struct type_constant<Type> : types::integral_constant<fmt_type, constant> {}

    TYPE_CONSTANT(char, fmt_type::S64);
    TYPE_CONSTANT(s32, fmt_type::S64);
    TYPE_CONSTANT(s64, fmt_type::S64);
    TYPE_CONSTANT(u32, fmt_type::U64);
    TYPE_CONSTANT(u64, fmt_type::U64);
    TYPE_CONSTANT(bool, fmt_type::Bool);
    TYPE_CONSTANT(f64, fmt_type::F64);
    TYPE_CONSTANT(string, fmt_type::String);
    TYPE_CONSTANT(const void *, fmt_type::Pointer);
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
    // 	void format(my_type val, void *data) {
    //      auto *f = (format_context *) data;
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

    template <typename T>
    void format_custom_arg(void *f, const void *arg) {
        formatter<types::remove_cvref_t<T>> formatter;
        formatter.format(*(const T *) (arg), f);
    }

    // Contains a value of any type
    struct value {
        struct custom {
            void *Data;
            void (*FormatFunc)(void *f, const void *arg);
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
        value(f64 value) : F64(value) {}
        value(const void *value) : Pointer(value) {}
        value(const string &value) : String(value) {}

        template <typename T>
        value(const T *value) {
            Custom.Data = (void *) value;
            Custom.FormatFunc = format_custom_arg<T>;
        }
    };

    // Holds a type and a value. If the value is not arithmetic (custom or string type)
    // then the life time of the parameter isn't extended (we just hold a pointer)!
    // That means that the parameters need to outlive the format function itself.
    struct fmt_arg {
        fmt_type Type = fmt_type::None;
        value Value;
    };

    // Maps formatting arguments to types that can be used to construct a fmt_value.
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
    auto fmt_map_arg(const U &val) {
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
            return (s64) val;
        } else if constexpr (types::is_signed_integral<T>) {
            return (s64) val;
        } else if constexpr (types::is_unsigned_integral<T>) {
            return (u64) val;
        } else if constexpr (types::is_floating_point<T>) {
            return (f64) val;
        } else if constexpr (types::is_same<T, string::code_point_ref>) {
            return (u64) val;
        } else if constexpr (types::is_enum<T>) {
            return fmt_map_arg((types::underlying_type_t<T>) val);
        } else {
            static_assert(false, "Argument doesn't have a formatter");
        }
    }

    // !!!
    // If you get a compiler error here it's probably because you passed in an argument that can't be formatted
    // To format custom types, implement a formatter specialization.
    // !!!
    template <typename T>
    constexpr auto fmt_mapped_type_constant_v = type_constant_v<decltype(fmt_map_arg(types::declval<T>()))>;

    template <typename T>
    fmt_arg fmt_make_arg(const T &v) { return {fmt_mapped_type_constant_v<T>, value(fmt_map_arg(v))}; }

    template <bool IsPacked, typename T>
    auto fmt_make_arg(const T &v) {
        using typoed = decltype(fmt_map_arg(types::declval<T>()));
        if constexpr (IsPacked) {
            return value(fmt_map_arg(v));  // We either pack values (we know their types in the fmg_args array)
        } else {
            return fmt_make_arg(v);  // .. or we don't have enough bits in the integer for everybody's type
                                     // and we store an array of fmt_args instead
        }
    }

    // Visits an argument dispatching with the right value based on the argument type
    template <typename Visitor>
    auto fmt_visit_fmt_arg(Visitor && visitor, const fmt_arg &ar)->decltype(visitor(0)) {
        switch (ar.Type) {
            case fmt_type::None:
                break;
            case fmt_type::S64:
                return visitor(ar.Value.S64);
            case fmt_type::U64:
                return visitor(ar.Value.U64);
            case fmt_type::Bool:
                return visitor(ar.Value.S64 != 0);
            case fmt_type::F64:
                return visitor(ar.Value.F64);
            case fmt_type::String:
                return visitor(ar.Value.String);
            case fmt_type::Pointer:
                return visitor(ar.Value.Pointer);
            case fmt_type::Custom:
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
        return (u64) fmt_mapped_type_constant_v<Arg> | (get_packed_fmt_types<Dummy, Args...>() << 4);
    }
    }  // namespace internal

    // We can't really combine this with _args_, ugh!
    // Stores either an array of values or arguments on the stack (just values if number is less than FMT_MAX_PACKED_ARGS)
    template <typename... Args>
    struct fmt_args_on_the_stack {
        static constexpr s64 NUM_ARGS = sizeof...(Args);
        static constexpr bool IS_PACKED = NUM_ARGS < FMT_MAX_PACKED_ARGS;

        using T = types::select_t<IS_PACKED, value, fmt_arg>;
        stack_array<T, NUM_ARGS> Data;

        u64 Types;

        fmt_args_on_the_stack(Args &&...args) : Types(IS_PACKED ? internal::get_packed_fmt_types<types::unused, Args...>() : FMT_IS_UNPACKED_BIT | NUM_ARGS) {
            Data = {fmt_make_arg<IS_PACKED>(args)...};
        }
    };

    struct fmt_args {
        void *Data;  // (value *) or (arg *) if not packed
        s64 Count = 0;
        u64 Types = 0;

        fmt_args() {}

        template <typename... Args>
        fmt_args(const fmt_args_on_the_stack<Args...> &store) : Data((void *) store.Data.Data), Types(store.Types), Count(sizeof...(Args)) {}
    };
}
