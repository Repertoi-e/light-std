#include "common.h"

#if defined OS_LINUX || defined OS_MAC

#include "format/console_colors.hpp"
#include "memory/array.hpp"
#include "string/string.hpp"

#include <stdlib.h>

CPPU_BEGIN_NAMESPACE

b32 fmt::internal::does_terminal_support_color() {
    const char *env = getenv("TERM");
    if (!env) return false;

    auto terms = to_array(string_view("ansi"), "color", "console", "cygwin", "gnome", "konsole", "kterm", "linux",
                          "msys", "putty", "rxvt", "screen", "vt100", "xterm");
    return terms.has(env);
}

CPPU_END_NAMESPACE

#endif