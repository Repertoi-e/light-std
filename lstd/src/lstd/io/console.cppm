module;

#include "../common.h"

export module lstd.console;

export import lstd.writer;

LSTD_BEGIN_NAMESPACE

export {
    struct console : writer {
        // By default, we are thread-safe.
        // If you don't use seperate threads and aim for maximum console output performance, set this to false.
        bool LockMutex = true;

        char *Buffer = null, *Current = null;
        s64 Available = 0, BufferSize = 0;

        enum output_type {
            COUT,
            CERR
        };
        output_type OutputType;
        console(output_type type) : OutputType(type) {}

        // Defined in os.*platform*.common.cppm
        void write(const char *data, s64 size) override;
        void flush() override;
    };

    inline auto cout = console(console::COUT);
    inline auto cerr = console(console::CERR);
}

LSTD_END_NAMESPACE
