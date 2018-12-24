#include "format/console_colors.hpp"

#if OS == WINDOWS

CPPU_BEGIN_NAMESPACE

// All windows terminals support colors
bool fmt::internal::does_terminal_support_color() { return true; }

CPPU_END_NAMESPACE

#endif