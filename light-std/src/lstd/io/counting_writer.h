#pragma once

#include "writer.h"

LSTD_BEGIN_NAMESPACE

namespace io {

void counting_writer_write(writer *w, const char *data, size_t count);

// This writer counts how many bytes have been written to it.
struct counting_writer : writer {
    size_t Count = 0;

    counting_writer() : writer(counting_writer_write, writer_flush_do_nothing) {}
};

inline void counting_writer_write(writer *w, const char *, size_t count) {
    auto *cw = (counting_writer *) w;
    cw->Count += count;
}

}  // namespace io

LSTD_END_NAMESPACE