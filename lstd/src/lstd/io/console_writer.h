#pragma once

#include "writer.h"

LSTD_BEGIN_NAMESPACE

struct console_writer : writer {
    // By default, we are thread-safe.
    // If you don't use seperate threads and aim for maximum console output performance, set this to false.
    bool LockMutex = true;

    byte *Buffer  = null, *Current = null;
    s64 Available = 0, BufferSize  = 0;

    enum output_type {
        COUT,
        CERR
    };

    output_type OutputType;

    console_writer() {
    }

    console_writer(output_type type)
        : OutputType(type) {
    }

    // Defined in *platform*_common.cpp
    void write(const byte *data, s64 size) override;
    void flush() override;
};

inline auto cout = console_writer(console_writer::COUT);
inline auto cerr = console_writer(console_writer::CERR);

LSTD_END_NAMESPACE
