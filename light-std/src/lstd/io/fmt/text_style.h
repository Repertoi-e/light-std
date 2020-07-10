#pragma once

#include "../../memory/string_utils.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

// Colors are defined all-uppercase.
// This enum contains names for popular colors and values for them in hex.
enum class color : u32 {
#define COLOR_DEF(x, y) x = y,
#include "colors.def"
#undef COLOR_DEF
};

inline string color_to_string(color c) {
    switch (c) {
#define COLOR_DEF(x, y) \
    case color::x:      \
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
inline color string_to_color(const string &str) {
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

inline string terminal_color_to_string(terminal_color c) {
    switch (c) {
#define COLOR_DEF(x, y)     \
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
inline terminal_color string_to_terminal_color(const string &str) {
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
    } Color{};
    bool Background = false;
    emphasis Emphasis = (emphasis) 0;
};

namespace internal {
// Used when making ANSII escape codes for text styles
inline char *u8_to_esc(char *p, char delimiter, u8 c) {
    *p++ = '0' + c / 100;
    *p++ = '0' + c / 10 % 10;
    *p++ = '0' + c % 10;
    *p++ = delimiter;
    return p;
}

inline char *color_to_ansii(char *buffer, text_style style) {
    char *p = buffer;
    if (style.ColorKind != text_style::color_kind::NONE) {
        if (style.ColorKind == text_style::color_kind::TERMINAL) {
            // Background terminal colors are 10 more than the foreground ones
            u32 value = (u32) style.Color.Terminal + (style.Background ? 10 : 0);

            *p++ = '\x1b';
            *p++ = '[';

            if (value >= 100) {
                *p++ = '1';
                value %= 100;
            }
            *p++ = '0' + value / 10;
            *p++ = '0' + value % 10;

            *p++ = 'm';
        } else {
            copy_memory(p, style.Background ? "\x1b[48;2;" : "\x1b[38;2;", 7);
            p += 7;

            p = u8_to_esc(p, ';', (u8)((style.Color.RGB >> 16) & 0xFF));
            p = u8_to_esc(p, ';', (u8)((style.Color.RGB >> 8) & 0xFF));
            p = u8_to_esc(p, 'm', (u8)((style.Color.RGB) & 0xFF));
        }
    } else if ((u8) style.Emphasis == 0) {
        // Empty text style means "reset"
        copy_memory(p, "\x1b[0m", 4);
        p += 4;
    }
    return p;
}

inline char *emphasis_to_ansii(char *buffer, u8 emphasis) {
    u8 codes[4] = {};
    if (emphasis & (u8) emphasis::BOLD) codes[0] = 1;
    if (emphasis & (u8) emphasis::ITALIC) codes[1] = 3;
    if (emphasis & (u8) emphasis::UNDERLINE) codes[2] = 4;
    if (emphasis & (u8) emphasis::STRIKETHROUGH) codes[3] = 9;

    char *p = buffer;
    For(range(4)) {
        if (!codes[it]) continue;

        *p++ = '\x1b';
        *p++ = '[';
        *p++ = '0' + codes[it];
        *p++ = 'm';
    }
    return p;
}
}  // namespace internal
}  // namespace fmt

LSTD_END_NAMESPACE
