#include "common.hpp"

#if OS == LINUX || OS == MAC

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

void *linux_allocator(Allocator_Mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, s32) {
    switch (mode) {
        case Allocator_Mode::ALLOCATE:
            [[fallthrough]];
        case Allocator_Mode::RESIZE:
            void *result;
            if (mode == Allocator_Mode::ALLOCATE) {
                result = mmap(null, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
            } else {
                result = mremap(oldMemory, oldSize, size, MREMAP_MAYMOVE);
            }
            if (result == (void *) -1) return 0;
            return result;
        case Allocator_Mode::FREE:
            munmap(oldMemory, oldSize);
            return null;
        case Allocator_Mode::FREE_ALL:
            return null;
        default:
            assert(false);  // We shouldn't get here
    }
    return null;
}

Allocator_Func DefaultAllocator = linux_allocator;

void os_exit_program(int code) { _exit(code); }

void os_assert_failed(const char *file, int line, const char *condition) {
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
    if (str.ByteLength > Available) {
        flush();
    }

    copy_memory(Current, str.Data, str.ByteLength);
    Current += str.ByteLength;
    Available -= str.ByteLength;
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
        if (string_view(env).begins_with(it)) {
            return true;
        }
    }
    return false;
}

CPPU_END_NAMESPACE

#endif  // OS == LINUX || OS == MAC