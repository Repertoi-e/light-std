#pragma once

#include "../internal/common.h"
#include "../memory/string.h"

LSTD_BEGIN_NAMESPACE

namespace io {

struct writer;
inline void writer_flush_do_nothing(writer *) {}

// Provides a way to write types and bytes with a simple extension API.
// Holds a pointer to _write_t_ and _flush_t_. Every other function
// in this class is implemented by calling those functions.
// By default _FlushFunction_ points to a stub that does nothing.
struct writer : non_copyable, non_movable, non_assignable {
    using write_t = void (*)(writer *w, const byte *data, s64 count);
    using flush_t = void (*)(writer *w);

    write_t WriteFunction = null;
    flush_t FlushFunction = writer_flush_do_nothing;

    byte *Buffer = null, *Current = null;
    s64 BufferSize = 0, Available = 0;

    writer() {}
    writer(write_t writeFunction, flush_t flushFunction) : WriteFunction(writeFunction), FlushFunction(flushFunction) {}

    void write(const array<byte> &data) { WriteFunction(this, data.Data, data.Count); }
    void write(const byte *data, s64 size) { WriteFunction(this, data, size); }
    
    void write(const string &str) { WriteFunction(this, (byte *) str.Data, str.Count); }

    void writer::write(utf32 cp) {
        utf8 data[4];
        encode_cp(data, cp);
        WriteFunction(this, (byte *) data, get_size_of_cp(data));
    }

    void flush() { FlushFunction(this); }
};

}  // namespace io

LSTD_END_NAMESPACE
