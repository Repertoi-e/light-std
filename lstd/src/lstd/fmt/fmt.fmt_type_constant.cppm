module;

#include "../common.h"

export module lstd.fmt.fmt_type_constant;

export import lstd.fmt.fmt_type;
export import lstd.string;

LSTD_BEGIN_NAMESPACE

export {
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
}

LSTD_END_NAMESPACE
