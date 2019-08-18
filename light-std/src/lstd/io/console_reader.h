#pragma once

#include "reader.h"

LSTD_BEGIN_NAMESPACE

namespace io {

// Defined in *platform*_common.cpp
char console_reader_request_byte(reader *r);

struct console_reader : reader, non_copyable, non_movable {
    // By default, we are thread-safe.
    // If you don't use seperate threads and aim for max performance, set this to false.
    bool LockMutex = true;

    console_reader() : reader(console_reader_request_byte) {}
};

// Standard input. Normally thread safe, optionally not (set LockMutex flag to false)
inline console_reader cin;

}  // namespace io

LSTD_END_NAMESPACE
