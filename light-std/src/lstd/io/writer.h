#pragma once

#include "../internal/common.h"
#include "../memory/string.h"

LSTD_BEGIN_NAMESPACE

namespace io {

struct writer;
inline void writer_flush_do_nothing(writer *) {}

// @Temp ??
// template <typename T>
// bool serialize(T *src, fmt::foromt*f);

// Provides a way to write types and bytes with a simple extension API.
// Holds a pointer to _write_t_ and _flush_t_. Every other function
// in this class is implemented by calling those functions.
// By default _FlushFunction_ points to a stub that does nothing.
struct writer : non_copyable, non_movable, non_assignable {
    using write_t = void (*)(writer *w, const char *data, s64 count);
    using flush_t = void (*)(writer *w);

    write_t WriteFunction = null;
    flush_t FlushFunction = writer_flush_do_nothing;

    char *Buffer = null, *Current = null;
    s64 BufferSize = 0, Available = 0;

    writer() = default;
    writer(write_t writeFunction, flush_t flushFunction) : WriteFunction(writeFunction), FlushFunction(flushFunction) {}

    void write(array_view<char> data) { WriteFunction(this, data.begin(), data.size()); }
    void write(const char *data) { WriteFunction(this, data, c_string_length(data)); }
    void write(const char *data, s64 count) { WriteFunction(this, data, count); }
    void write(const string &str) { WriteFunction(this, str.Data, str.ByteLength); }

    void write(char32_t cp) {
        char data[4];
        encode_cp(data, cp);
        WriteFunction(this, data, get_size_of_cp(data));
    }

    void flush() { FlushFunction(this); }
};

}  // namespace io

LSTD_END_NAMESPACE
