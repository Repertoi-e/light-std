#pragma once

#include "writer.h"

LSTD_BEGIN_NAMESPACE

namespace io {

// Defined in *platform*_common.cpp
void console_writer_write(writer *w, const byte *data, s64 count);
void console_writer_flush(writer *w);

struct console_writer : writer {
    // By default, we are thread-safe.
    // If you don't use seperate threads and aim for maximum console output performance, set this to false.
    bool LockMutex = true;

    enum output_type { COUT,
                       CERR };
    output_type OutputType;

    console_writer(output_type type) : writer(console_writer_write, console_writer_flush), OutputType(type) {}
};

inline auto cout = console_writer(console_writer::COUT);
inline auto cerr = console_writer(console_writer::CERR);

}  // namespace io

LSTD_END_NAMESPACE
