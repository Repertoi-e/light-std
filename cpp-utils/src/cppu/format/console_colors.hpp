#pragma once

#include "core.hpp"

CPPU_BEGIN_NAMESPACE

namespace fmt {

enum class Style {
    Reset = 0,
    Bold = 1,
    Dim = 2,
    Italic = 3,
    Underline = 4,
    Blink = 5,
    Rblink = 6,
    Reversed = 7,
    Conceal = 8,
    Crossed = 9
};

enum class FG {
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    Gray = 37,
    Reset = 39
};

enum class BG {
    Black = 40,
    Red = 41,
    Green = 42,
    Yellow = 43,
    Blue = 44,
    Magenta = 45,
    Cyan = 46,
    Gray = 47,
    Reset = 49
};

enum class FGB { Black = 90, Red = 91, Green = 92, Yellow = 93, Blue = 94, Magenta = 95, Cyan = 96, Gray = 97 };
enum class BGB { Black = 100, Red = 101, Green = 102, Yellow = 103, Blue = 104, Magenta = 105, Cyan = 106, Gray = 107 };

namespace internal {
b32 does_terminal_support_color();
}

template <typename T>
struct Formatter<T, typename std::enable_if_t<std::is_same_v<T, fmt::Style> || std::is_same_v<T, fmt::FG> ||
                                              std::is_same_v<T, fmt::BG> || std::is_same_v<T, fmt::FGB> ||
                                              std::is_same_v<T, fmt::BGB>>> {
    void format(const T &value, Format_Context &f) {
        if (internal::does_terminal_support_color()) {
            f.write("\033[");
            f.write_int((s32) value);
            f.write('m');
        }
    }
};
}  // namespace fmt

CPPU_END_NAMESPACE