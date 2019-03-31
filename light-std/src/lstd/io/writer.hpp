#pragma once

#include "../common.hpp"

#include "../memory/memory_buffer.hpp"
#include "../string/string_builder.hpp"

#include "../thread.hpp"

LSTD_BEGIN_NAMESPACE

struct string;
namespace io {
struct writer;
}

namespace fmt::internal {
template <typename... Args>
void to_writer(io::writer &writer, const string_view &formatString, Args &&... args);
}

namespace io {

inline void writer_flush_do_nothing(void *data) {}

// Provides a simple API to write stuff.
// Any subclass needs to provide pointers for the functions write(const Memory_View &) and flush()
// All other overloads are implemented around those functions.
// Note: A default flush function which does nothing is provided by default.
struct writer {
    byte *Buffer = null, *Current = null;
    size_t Available = 0;

    using write_type = void (*)(void *data, const memory_view &memory);
    using flush_type = void (*)(void *data);

    write_type WriteFunction = null;
    flush_type FlushFunction = writer_flush_do_nothing;

    void flush() { FlushFunction(this); }
    void write(const memory_view &memory) { WriteFunction(this, memory); }
    void write(const byte *data, size_t size) { WriteFunction(this, memory_view(data, size)); }

    // TODO: Change to just write(ch)
    void write_codepoint(char32_t ch) {
        byte data[4];
        encode_code_point(data, ch);
        WriteFunction(this, memory_view(data, get_size_of_code_point(data)));
    }

    // Remove all trailing bytes that equal _ch_.
    // This function does not notify any implementations
    // so for example Counter_Writer doesn't account for these "unwritten" bytes
    void remove_trailing_bytes(byte ch) {
        while (Current != Buffer && *(Current - 1) == ch) {
            --Current;
            Available++;
        }
    }

    template <typename... Args>
    void write_fmt(const string_view &formatString, Args &&... args) {
        fmt::internal::to_writer(*this, formatString, std::forward<Args>(args)...);
    }
};

// A Writer around String_Builder
void string_writer_write(void *data, const memory_view &memory);

struct string_writer : writer {
    string_builder Builder;

    string_writer() { WriteFunction = string_writer_write; }
};

void console_writer_write(void *data, const memory_view &memory);
void console_writer_flush(void *data);

struct console_writer : writer, NonCopyable, NonMovable {
    // By default, we are thread-safe.
    // If you don't use seperate threads and aim for max
    // performance, set this to false.
    bool LockMutex = true;

    console_writer();
    console_writer(bool cerr);

   private:
    bool _Err = false;
    thread::recursive_mutex *_Mutex = null;

    friend void console_writer_write(void *, const memory_view &);
    friend void console_writer_flush(void *);
};

inline auto cout = console_writer(false);
inline auto cerr = console_writer(true);

// Writer that does nothing but count how many bytes would have been written
void counter_writer_write(void *data, const memory_view &memory);

struct counter_writer : writer {
    size_t Count = 0;

    counter_writer() { WriteFunction = counter_writer_write; }
};

// Writer around a Memory_Buffer
template <size_t S>
void memory_buffer_write(void *data, const memory_view &memory);

template <size_t S>
struct memory_buffer_writer : writer {
    memory_buffer<S> &Buffer;

    memory_buffer_writer(memory_buffer<S> &buffer) : Buffer(buffer) { WriteFunction = memory_buffer_write<S>; }
};

template <size_t S>
void memory_buffer_write(void *data, const memory_view &memory) {
    auto *writer = (memory_buffer_writer<S> *) data;
    writer->Buffer.append(memory);
}

}  // namespace io

LSTD_END_NAMESPACE
