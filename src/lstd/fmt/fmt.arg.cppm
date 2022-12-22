module;

#include "../common.h"

export module lstd.fmt.arg;

export import lstd.fmt.fmt_type;
import lstd.fmt.fmt_type_constant;

LSTD_BEGIN_NAMESPACE

export {
    struct fmt_custom_value {
        void *Data;
        void (*FormatFunc)(void *formatContext, void *arg);
    };

    // Contains a value of any type
    struct fmt_value {
        union {
            s64 S64;
            u64 U64;
            f32 F32;
            f64 F64;

            void *Pointer;
            string String;

            fmt_custom_value Custom;
        };

        fmt_value(s64 v = 0) : S64(v) {}
        fmt_value(bool v) : S64(v) {}  // We store bools in S64
        fmt_value(u64 v) : U64(v) {}
        fmt_value(f32 v) : F32(v) {}
        fmt_value(f64 v) : F64(v) {}
        fmt_value(void *v) : Pointer(v) {}
        fmt_value(string v) : String(v) {}

        // Attempt to call a custom formatter.
        // Compile-time asserts if there was no overload.
        template <typename T>
        fmt_value(T *v) {
            Custom.Data       = (void *) v;
            Custom.FormatFunc = call_write_on_custom_arg<T>;
        }

        template <typename T>
        static void call_write_on_custom_arg(void *formatContext, void *arg) {
            write_custom((fmt_context *) formatContext, (const T *) arg);
        }
    };

    // Holds a type and a value. If the value is not arithmetic (custom or string type)
    // then the life time of the parameter isn't extended (we just hold a pointer)!
    // That means that the parameters need to outlive the parse and format function itself.
    struct fmt_arg {
        fmt_type Type = fmt_type::NONE;
        fmt_value Value;
    };

    // Maps formatting arguments to types that can be used to construct a fmt_value.
    //
    // The order in which we look:
    //   * is string constructible from T? then we map to string(T)
    //   * is the type a code_point_ref? maps to u64 (we want the value in that case)
    //   * is the type an (unsigned) integral? maps to (u64) s64
    //   * is the type an enum? calls map_arg again with the underlying type
    //   * is the type a floating point? maps to f64
    //   * is the type a pointer? if it's non-void we throw an error, otherwise we map to (void *) v
    //   * is the type a bool? maps to bool
    //   * otherwise maps to &v (value then setups a function call to a custom formatter)
    auto fmt_map_arg(auto no_copy v) {
        using T = remove_cvref_t<decltype(v)>;

        if constexpr (is_same<string, T> || is_constructible<string, T>) {
            return string(v);
        } else if constexpr (is_same<T, string::code_point_ref>) {
            return (u64) v;
        } else if constexpr (is_same<bool, T>) {
            return v;
        } else if constexpr (is_unsigned_integral<T>) {
            return (u64) v;
        } else if constexpr (is_signed_integral<T>) {
            return (s64) v;
        } else if constexpr (is_enum<T>) {
            return fmt_map_arg((underlying_type_t<T>) v);
        } else if constexpr (is_floating_point<T>) {
            return v;
        } else if constexpr (is_pointer<T>) {
            static_assert(is_same<T, void *>, "Formatting of non-void pointers is disallowed");
            return v;
        } else {
            return &v;
        }
    }

    template <typename T>
    auto fmt_mapped_type_constant_v = type_constant_v<decltype(fmt_map_arg(declval<T>()))>;

    fmt_arg fmt_make_arg(auto no_copy v) { return {fmt_mapped_type_constant_v<decltype(v)>, fmt_value(fmt_map_arg(v))}; }

    // Visits an argument dispatching with the right value based on the argument type
    template <typename Visitor>
    auto fmt_visit_arg(Visitor visitor, fmt_arg ar)->decltype(visitor(0)) {
        switch (ar.Type) {
            case fmt_type::NONE:
                break;
            case fmt_type::S64:
                return visitor(ar.Value.S64);
            case fmt_type::U64:
                return visitor(ar.Value.U64);
            case fmt_type::BOOL:
                return visitor(ar.Value.S64 != 0);  // We store bools in S64
            case fmt_type::F32:
                return visitor(ar.Value.F32);
            case fmt_type::F64:
                return visitor(ar.Value.F64);
            case fmt_type::STRING:
                return visitor(ar.Value.String);
            case fmt_type::POINTER:
                return visitor(ar.Value.Pointer);
            case fmt_type::CUSTOM:
                return visitor(ar.Value.Custom);
        }
        return visitor(unused{});
    }
}

LSTD_END_NAMESPACE
