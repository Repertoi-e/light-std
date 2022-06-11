module;

#include "../common.h"

export module lstd.writer;

export import lstd.string;

LSTD_BEGIN_NAMESPACE

export {
    // Provides a way to write types and bytes with a simple extension API.
    // Subclasses of this stuct override the write/flush methods depending on the output (console, files, buffers, etc.)
    // Types are written with the _write_ overloads outside of this struct.
    struct writer {
        virtual void write(const char *data, s64 count) = 0;
        virtual void flush() {}
    };

    void write(writer * w, string str) { w->write(str.Data, str.Count); }
    void write(writer * w, const char *data, s64 size) { w->write(data, size); }
    void write(writer * w, code_point cp) {
        char data[4];
        utf8_encode_cp(data, cp);
        w->write(data, utf8_get_size_of_cp(data));
    }

    // For printing and formatting more types see the lstd.fmt module.

    void flush(writer * w) { w->flush(); }

    // Doesn't do anything but count how much bytes would have been written to it.
    // Used in the lstd.fmt module to calculate formatted length.
    struct counting_writer : writer {
        s64 Count = 0;

        void write(const char *data, s64 count) override { Count += count; }
        void flush() override {}
    };
}

LSTD_END_NAMESPACE
