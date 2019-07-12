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

inline string_view color_to_string(color c) {
    switch (c) {
#define COLOR_DEF(x, y) \
    case color::##x:    \
        return #x;
#include "colors.def"
#undef COLOR_DEF
        default:
            return "UNKNOWN";
    }
}

// Colors are defined all-uppercase and this function is case-sensitive
//   e.g. cornflower_blue doesn't return color::CORNFLOWER_BLUE
// Returns WHITE if not found.
inline color string_to_color(string_view str) {
#define COLOR_DEF(x, y) \
    if (str == #x) return color::##x;
#include "colors.def"
#undef COLOR_DEF
    return color::WHITE;
}

// Colors are defined all-uppercase.
// Terminal colors are meant to be used if the output console doesn't support true color.
// There are 8 colors with additional 8 bright versions.
enum class terminal_color : u32 {
#define COLOR_DEF(x, y) x = y,
#include "terminal_colors.def"
#undef COLOR_DEF
};

inline string_view terminal_color_to_string(terminal_color c) {
    switch (c) {
#define COLOR_DEF(x, y)       \
    case terminal_color::##x: \
        return #x;
#include "terminal_colors.def"
#undef COLOR_DEF
        default:
            return "UNKNOWN";
    }
}

// Colors are defined all-uppercase and this function is case-sensitive
//   e.g. bright_black doesn't return color::BRIGHT_BLACK
// Returns WHITE if not found.
inline terminal_color string_to_terminal_color(string_view str) {
#define COLOR_DEF(x, y) \
    if (str == #x) return color::##x;
#include "terminal_colors.def"
#undef COLOR_DEF
    return terminal_color::WHITE;
}

struct fmt_color {
    enum class kind { RGB = 0, TERMINAL };

    kind Kind;
    union {
        terminal_color COLOR;
        u32 RGB;
    } Color;
};

}  // namespace fmt

LSTD_END_NAMESPACE
