module;

#include "../memory/string.h"

export module fmt.fmt_type;

LSTD_BEGIN_NAMESPACE

export {
    enum class fmt_type {
        NONE = 0,

        S64,
        U64,
        BOOL,
        LAST_INTEGRAL = BOOL,

        F32,
        F64,
        LAST_ARITHMETIC = F64,

        STRING,
        POINTER,

        CUSTOM
    };

    constexpr bool fmt_is_type_integral(fmt_type type) {
        return type > fmt_type::NONE && type <= fmt_type::LAST_INTEGRAL;
    }

    constexpr bool fmt_is_type_arithmetic(fmt_type type) {
        return type > fmt_type::NONE && type <= fmt_type::LAST_ARITHMETIC;
    }

    namespace fmt_internal {

    constexpr u64 IS_UNPACKED_BIT = 1ull << 63;
    constexpr u32 MAX_PACKED_ARGS = 15;

    template <typename T>
    struct type_constant : types::integral_constant<fmt_type, fmt_type::CUSTOM> {};

#define TYPE_CONSTANT(Type, constant) \
    template <>                       \
    struct type_constant<Type> : types::integral_constant<fmt_type, constant> {}

    TYPE_CONSTANT(char, fmt_type::S64);
    TYPE_CONSTANT(s32, fmt_type::S64);
    TYPE_CONSTANT(s64, fmt_type::S64);
    TYPE_CONSTANT(u32, fmt_type::U64);
    TYPE_CONSTANT(u64, fmt_type::U64);
    TYPE_CONSTANT(bool, fmt_type::BOOL);
    TYPE_CONSTANT(f32, fmt_type::F32);
    TYPE_CONSTANT(f64, fmt_type::F64);
    TYPE_CONSTANT(string, fmt_type::STRING);
    TYPE_CONSTANT(const void *, fmt_type::POINTER);
#undef TYPE_CONSTANT

    template <typename T>
    constexpr auto type_constant_v = type_constant<types::remove_cvref_t<T>>::value;
    }  // namespace fmt_internal
}

LSTD_END_NAMESPACE
