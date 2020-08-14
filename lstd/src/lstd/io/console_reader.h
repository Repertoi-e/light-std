#pragma once

#include "reader.h"

LSTD_BEGIN_NAMESPACE

namespace io {

// Defined in *platform*_common.cpp
char console_reader_give_me_buffer(reader *r);

// This reader implements standard console input.
// Also provides some extra functions.
//
// Readers aren't thread-safe. That's the callers responsibility! (why would you want to read from multiple threads anyway... that seems weird?).
struct console_reader : reader {
    console_reader() : reader(console_reader_give_me_buffer) {}
};

// Standard input. Not thread-safe (why would you want to read from multiple threads anyway... that seems weird?).
inline console_reader cin;

}  // namespace io

LSTD_END_NAMESPACE
