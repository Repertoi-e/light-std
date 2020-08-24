#pragma once

#include "writer.h"

LSTD_BEGIN_NAMESPACE

namespace io {

void counting_writer_write(writer *w, const byte *data, s64 size);

// This writer counts how many bytes would have been written. The actual data is discarded.
struct counting_writer : writer {
    s64 Count = 0;

    counting_writer() : writer(counting_writer_write, writer_flush_do_nothing) {}
};

inline void counting_writer_write(writer *w, const byte *, s64 size) {
    auto *cw = (counting_writer *) w;
    cw->Count += size;
}

}  // namespace io

LSTD_END_NAMESPACE
