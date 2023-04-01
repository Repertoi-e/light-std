#pragma once

// Use this to get the location where a function
// was called without using macros.

#if defined LSTD_NO_CRT

// @Platform Compiles on MSVC only. @Robustness Not fully compatible with
// std::source_location (missing column info).
struct source_location {
  const char *File = "Unknown";
  const char *Function = "Unknown";
  int Line = 0;

  constexpr source_location() {}

  // Uses built-in compiler functions.
  static consteval source_location current(
      const char *file = __builtin_FILE(),
      const char *func = __builtin_FUNCTION(), int line = __builtin_LINE()) {
    source_location loc;
    loc.File = file;
    loc.Function = func;
    loc.Line = line;
    return loc;
  }
};
#else
#include <source_location>

LSTD_BEGIN_NAMESPACE
using source_location = std::source_location;
LSTD_END_NAMESPACE

#endif
