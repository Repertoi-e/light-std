#pragma once

#include "../scalar_types.h"

// Use this to get the location where a function was called without using macros.
// Uses built-in compiler functions. @TODO: Works on MSVC. Should work on GNU and Clang?
struct source_location {
    const char *File     = "Unknown";
    const char *Function = "Unknown";
    s64 Line             = 0;

    constexpr source_location() {}

    static constexpr source_location current(const char *file = __builtin_FILE(), const char *func = __builtin_FUNCTION(), s64 line = __builtin_LINE()) {
        source_location loc;
        loc.File     = file;
        loc.Function = func;
        loc.Line     = line;
        return loc;
    }
};
