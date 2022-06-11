#pragma once

#include "../platform.h"

// Replacement for the std::is_constant_evaluated
#if COMPILER == MSVC
[[nodiscard]] constexpr bool is_constant_evaluated() noexcept { return __builtin_is_constant_evaluated(); }
#else
#error Implement.
#endif
