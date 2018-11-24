#pragma once

#include "core.h"

#if !defined CPPU_NO_CRT
#if !defined CPPU_FMT_THOUSANDS_SEPARATOR
#include <locale>
#endif
#endif

CPPU_BEGIN_NAMESPACE

namespace fmt {

enum class Alignment { DEFAULT, LEFT, RIGHT, CENTER, NUMERIC };
enum class Flag : u32 { SIGN = 1, PLUS = 2, MINUS = 4, HASH = 8 };

inline Flag operator|(Flag lhs, Flag rhs) {
    using T = std::underlying_type_t<Flag>;
    return (Flag)((T) lhs | (T) rhs);
}
inline Flag &operator|=(Flag &lhs, Flag rhs) {
    using T = std::underlying_type_t<Flag>;
    lhs = (Flag)((T) lhs | (T) rhs);
    return lhs;
}

struct Align_Spec {
    u32 Width;
    char32_t Fill;
    Alignment Align;

    constexpr Align_Spec(u32 width, char32_t fill, Alignment align = Alignment::DEFAULT)
        : Width(width), Fill(fill), Align(align) {}
};

class Format_Specs : public Align_Spec {
   public:
    Flag Flags = (Flag) 0;
    s32 Precision = -1;
    char32_t Type;

    constexpr Format_Specs(u32 width = 0, char32_t type = 0, char32_t fill = ' ')
        : Align_Spec(width, fill), Type(type) {}
    constexpr b32 has_flag(Flag flag) const { return ((u32) Flags & (u32) flag) != 0; }
};

struct Argument_Ref {
    enum class Kind { NONE, INDEX, NAME };
    Kind Kind = Kind::NONE;
    union {
        u32 Index;
        string_view Name;
    };

    constexpr Argument_Ref() : Index(0) {}
    constexpr explicit Argument_Ref(u32 index) : Kind(Kind::INDEX), Index(index) {}
    constexpr explicit Argument_Ref(const string_view &name) : Kind(Kind::NAME), Name(name) {}

    constexpr Argument_Ref &operator=(u32 index) {
        Kind = Kind::INDEX;
        Index = index;
        return *this;
    }
};

struct Dynamic_Format_Specs : Format_Specs {
    Argument_Ref WidthRef;
    Argument_Ref PrecisionRef;
};
}  // namespace fmt

CPPU_END_NAMESPACE