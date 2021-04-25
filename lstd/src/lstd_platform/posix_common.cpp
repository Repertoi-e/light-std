#include "lstd/common.hpp"

// @Platform
// UNMAINTAINED
// UNMAINTAINED
// UNMAINTAINED

#if IS_OS_POSIX

#if defined LSTD_NO_CRT
#error LSTD_NO_CRT is Windows-only
#endif

#include "io.hpp"

#include "memory/array.hpp"
#include "string/string.hpp"

#include <sys/mman.h>
#include <sys/time.h>

#include <stdlib.h>

#include <termios.h>
#include <unistd.h>

#include <csignal>

CPPU_BEGIN_NAMESPACE

void os_exit_program(s32 code) { _exit(code); }

void os_assert_failed(const char *file, s64 line, const char *condition) {
    fmt::print("{}>>> {}:{}, Assert failed: {}{}\n", fmt::FG::Red, file, line, condition, fmt::FG::Reset);
#if COMPILER == GCC || COMPILER == CLANG
    std::raise(SIGINT);
#else
    os_exit_program(-1);
#endif
}

#define CONSOLE_BUFFER_SIZE 1_KiB

io::Console_Writer::Console_Writer() {
    Buffer = New<byte>(CONSOLE_BUFFER_SIZE);
    Current = Buffer;
    Available = CONSOLE_BUFFER_SIZE;
}

void io::Console_Writer::write(const Memory_View &str) {
    if (str.Count > Available) {
        flush();
    }

    copy_memory(Current, str.Data, str.Count);
    Current += str.Count;
    Available -= str.Count;
}

void io::Console_Writer::flush() {
    ::write(STDOUT_FILENO, Buffer, CONSOLE_BUFFER_SIZE - Available);

    Current = Buffer;
    Available = CONSOLE_BUFFER_SIZE;
}

io::Console_Reader::Console_Reader() {
    // Leak, but doesn't matter since the object is global
    Buffer = New<byte>(CONSOLE_BUFFER_SIZE);
    Current = Buffer;
}

byte io::Console_Reader::request_byte() {
    assert(Available == 0);  // Sanity

    size_t read = ::read(STDIN_FILENO, (char *) Buffer, CONSOLE_BUFFER_SIZE);

    Current = Buffer;
    Available = read;

    return (read == 0) ? io::eof : (*Current);
}

f64 os_get_wallclock_in_seconds() {
    timeval time;
    assert(!gettimeofday(&time, 0));
    return (f64) time.tv_sec + (f64) time.tv_usec * 0.000001;
}

bool fmt::internal::does_terminal_support_color() {
    const char *env = getenv("TERM");
    if (!env) return false;

    auto terms = to_array(string_view("ansi"), "color", "console", "cygwin", "gnome", "konsole", "kterm", "linux",
        "msys", "putty", "rxvt", "screen", "vt100", "xterm");
    For(terms) {
        if (string_view(env).match_beginning(it)) {
            return true;
        }
    }
    return false;
}

CPPU_END_NAMESPACE

#endif  // OS == LINUX || OS == MAC