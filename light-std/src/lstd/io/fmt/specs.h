#pragma once

#include "error_handler.h"
#include "text_style.h"
#include "value.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

enum class alignment { DEFAULT = 0, LEFT, RIGHT, CENTER, NUMERIC };
enum class flag : u32 { SIGN = BIT(0), PLUS = BIT(1), MINUS = BIT(2), HASH = BIT(3) };

constexpr flag operator|(flag lhs, flag rhs) {
    using T = underlying_type_t<flag>;
    return (flag)((T) lhs | (T) rhs);
}

constexpr flag &operator|=(flag &lhs, flag rhs) {
    using T = underlying_type_t<flag>;
    lhs = (flag)((T) lhs | (T) rhs);
    return lhs;
}

struct format_specs {
    char32_t Fill;
    alignment Align;

    flag Flags = (flag) 0;

    u32 Width;
    s32 Precision = -1;

    byte Type;

    constexpr bool has_flag(flag flag) const { return ((u32) Flags & (u32) flag) != 0; }
};

struct arg_ref {
    enum class kind { NONE = 0, INDEX, NAME };
    kind Kind = kind::NONE;
    union {
        u32 Index;
        string_view Name;
    };

    constexpr arg_ref() : Index(0) {}
    constexpr explicit arg_ref(u32 index) : Kind(kind::INDEX), Index(index) {}
    constexpr explicit arg_ref(string_view name) : Kind(kind::NAME), Name(name) {}

    constexpr arg_ref &operator=(u32 index) {
        Kind = kind::INDEX;
        Index = index;
        return *this;
    }
};

struct dynamic_format_specs : format_specs {
    arg_ref WidthRef;
    arg_ref PrecisionRef;
};
}  // namespace fmt

LSTD_END_NAMESPACE
