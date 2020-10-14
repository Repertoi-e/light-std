#pragma once

#include "writer.h"

LSTD_BEGIN_NAMESPACE

// This writer counts how many bytes would have been written. The actual data is discarded.
struct counting_writer : writer {
    s64 Count = 0;

    void write(const byte *, s64 size) override { Count += size; }
};

LSTD_END_NAMESPACE
