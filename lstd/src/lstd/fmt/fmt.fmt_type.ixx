module;

#include "../memory/string.h"

export module fmt.fmt_type;

LSTD_BEGIN_NAMESPACE

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

    namespace fmt_internal {

    constexpr u64 IS_UNPACKED_BIT = 1ull << 63;
    constexpr u32 MAX_PACKED_ARGS = 15;

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

    template <typename T>
    constexpr auto type_constant_v = type_constant<types::remove_cvref_t<T>>::value;
    }  // namespace fmt_internal
}

LSTD_END_NAMESPACE
