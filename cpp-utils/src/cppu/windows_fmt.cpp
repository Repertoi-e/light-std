#include "format/console_colors.h"

#if defined OS_WINDOWS

CPPU_BEGIN_NAMESPACE

// All windows terminals support colors
b32 fmt::internal::does_terminal_support_color() { return true; }

CPPU_END_NAMESPACE

#endif