#pragma once

#include "core.hpp"

LSTD_BEGIN_NAMESPACE

namespace fmt {

enum class style {
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

enum class fg {
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

enum class bg {
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

enum class fgb { Black = 90, Red = 91, Green = 92, Yellow = 93, Blue = 94, Magenta = 95, Cyan = 96, Gray = 97 };
enum class bgb { Black = 100, Red = 101, Green = 102, Yellow = 103, Blue = 104, Magenta = 105, Cyan = 106, Gray = 107 };

namespace internal {
bool does_terminal_support_color();
}

template <typename T>
struct formatter<T, std::enable_if_t<std::is_same_v<T, style> || std::is_same_v<T, fg> || std::is_same_v<T, bg> ||
                                     std::is_same_v<T, fgb> || std::is_same_v<T, bgb>>> {
    void format(const T &value, format_context &f) {
        if (internal::does_terminal_support_color()) {
            f.write("\033[");
            f.write_int((s32) value);
            f.write("m");
        }
    }
};
}  // namespace fmt

LSTD_END_NAMESPACE