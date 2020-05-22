#pragma once

#include "../memory/string.h"

LSTD_BEGIN_NAMESPACE

// Simple cross-platform wrapper around a dynamic library
struct dynamic_library : non_copyable, non_movable, non_assignable {
    void *Handle = null;

    dynamic_library() = default;
    dynamic_library(string name) { load(name); }
    ~dynamic_library() { close(); }

    bool load(string name);
    void close();

    void *get_symbol(string name);
};

LSTD_END_NAMESPACE
