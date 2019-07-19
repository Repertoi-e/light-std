#pragma once

#include "../../storage/string_utils.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {
namespace internal {
constexpr byte FG_COLOR[] = "\x1b[38;2;";
constexpr byte BG_COLOR[] = "\x1b[48;2;";
constexpr byte RESET_COLOR[] = "\x1b[0m";
}  // namespace internal

// Colors are defined all-uppercase.
// This enum contains names for popular colors and values for them in hex.
enum class color : u32 {
#define COLOR_DEF(x, y) x = y,
#include "colors.def"
#undef COLOR_DEF
};

constexpr string_view color_to_string(color c) {
    switch (c) {
#define COLOR_DEF(x, y) \
    case color::x:    \
        return #x;
#include "colors.def"
#undef COLOR_DEF
        default:
            return "UNKNOWN";
    }
}

// Colors are defined all-uppercase and this function is case-sensitive
//   e.g. cornflower_blue doesn't return color::CORNFLOWER_BLUE
// Returns color::NONE (with value of black) if not found.
constexpr color string_to_color(string_view str) {
#define COLOR_DEF(x, y) \
    if (str == #x) return color::x;
#include "colors.def"
#undef COLOR_DEF
    return color::NONE;
}

// Colors are defined all-uppercase.
// Terminal colors are meant to be used if the output console doesn't support true color.
// There are 8 colors with additional 8 bright versions.
enum class terminal_color : u32 {
#define COLOR_DEF(x, y) x = y,
#include "terminal_colors.def"
#undef COLOR_DEF
};

constexpr string_view terminal_color_to_string(terminal_color c) {
    switch (c) {
#define COLOR_DEF(x, y)       \
    case terminal_color::x: \
        return #x;
#include "terminal_colors.def"
#undef COLOR_DEF
        default:
            return "NONE";
    }
}

// Colors are defined all-uppercase and this function is case-sensitive
//   e.g. bright_black doesn't return color::BRIGHT_BLACK
// Returns terminal_color::NONE (invalid) if not found.
constexpr terminal_color string_to_terminal_color(string_view str) {
#define COLOR_DEF(x, y) \
    if (str == #x) return terminal_color::x;
#include "terminal_colors.def"
#undef COLOR_DEF
    return terminal_color::NONE;
}

enum class emphasis : u8 { BOLD = BIT(0), ITALIC = BIT(1), UNDERLINE = BIT(2), STRIKETHROUGH = BIT(3) };

constexpr emphasis operator|(emphasis lhs, emphasis rhs) {
    using T = underlying_type_t<emphasis>;
    return (emphasis)((T) lhs | (T) rhs);
}

constexpr emphasis &operator|=(emphasis &lhs, emphasis rhs) {
    using T = underlying_type_t<emphasis>;
    lhs = (emphasis)((T) lhs | (T) rhs);
    return lhs;
}

struct text_style {
    enum class color_kind { NONE = 0, RGB, TERMINAL };

    color_kind ColorKind = color_kind::NONE;
    union {
        u32 RGB = 0;
        terminal_color Terminal;
    } Color;
    bool Background = false;
    emphasis Emphasis = (emphasis) 0;

    constexpr text_style() = default;
};

}  // namespace fmt

LSTD_END_NAMESPACE
