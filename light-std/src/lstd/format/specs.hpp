#pragma once

#include "value.hpp"

#if !defined LSTD_NO_CRT && !defined LSTD_FMT_THOUSANDS_SEPARATOR
#include <locale>
#endif

LSTD_BEGIN_NAMESPACE

namespace fmt {

enum class alignment { DEFAULT, LEFT, RIGHT, CENTER, NUMERIC };
enum class flag : u32 { SIGN = 1, PLUS = 2, MINUS = 4, HASH = 8 };

inline flag operator|(flag lhs, flag rhs) {
    using T = std::underlying_type_t<flag>;
    return (flag)((T) lhs | (T) rhs);
}
inline flag &operator|=(flag &lhs, flag rhs) {
    using T = std::underlying_type_t<flag>;
    lhs = (flag)((T) lhs | (T) rhs);
    return lhs;
}

struct align_spec {
    u32 Width;
    char32_t Fill;
    alignment Align;

    constexpr align_spec(u32 width, char32_t fill, alignment align = alignment::DEFAULT)
        : Width(width), Fill(fill), Align(align) {}
};

class format_specs : public align_spec {
   public:
    flag Flags = (flag) 0;
    s32 Precision = -1;
    char32_t Type;

    constexpr format_specs(u32 width = 0, char32_t type = 0, char32_t fill = ' ')
        : align_spec(width, fill), Type(type) {}
    constexpr bool has_flag(flag flag) const { return ((u32) Flags & (u32) flag) != 0; }
};

struct argument_ref {
    enum class kind { NONE, INDEX, NAME };
    kind Kind = kind::NONE;
    union {
        u32 Index;
        string_view Name;
    };

    constexpr argument_ref() : Index(0) {}
    constexpr explicit argument_ref(u32 index) : Kind(kind::INDEX), Index(index) {}
    constexpr explicit argument_ref(const string_view &name) : Kind(kind::NAME), Name(name) {}

    constexpr argument_ref &operator=(u32 index) {
        Kind = kind::INDEX;
        Index = index;
        return *this;
    }
};

struct dynamic_format_specs : format_specs {
    argument_ref WidthRef;
    argument_ref PrecisionRef;
};
}  // namespace fmt

LSTD_END_NAMESPACE