#pragma once

#include "../memory/string.h"

LSTD_BEGIN_NAMESPACE

// Simple cross-platform wrapper around a dynamic library
struct dynamic_library : non_copyable, non_movable, non_assignable {
    void *Handle = null;

    dynamic_library() {}
    dynamic_library(const string &name) { load(name); }

    bool load(const string &name);
    void close();

    void *get_symbol(const string &name);
};

LSTD_END_NAMESPACE
