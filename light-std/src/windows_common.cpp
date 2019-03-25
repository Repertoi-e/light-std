#include "lstd/common.hpp"

#if OS == WINDOWS

#include "lstd/io.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

LSTD_BEGIN_NAMESPACE

void *os_memory_alloc(void *context, size_t size, size_t *outsize) { return HeapAlloc(GetProcessHeap(), 0, size); }
void os_memory_free(void *context, void *ptr) { HeapFree(GetProcessHeap(), 0, ptr); }

void *os_allocator(Allocator_Mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, uptr_t) {
    switch (mode) {
        case Allocator_Mode::ALLOCATE: {
            void *data = os_memory_alloc(0, size, 0);
            zero_memory(data, size);
            return data;
        }
        case Allocator_Mode::RESIZE:
            return HeapReAlloc(GetProcessHeap(), 0, oldMemory, size);
        case Allocator_Mode::FREE:
            os_memory_free(0, oldMemory);
            return null;
        case Allocator_Mode::FREE_ALL:
            assert(false);
            return null;
    }
    return null;
}

#if !defined LSTD_NO_CRT
void os_exit_program(int code) { exit(code); }
#endif

void os_assert_failed(const char *file, s32 line, const char *condition) {
    fmt::print("{}>>> {}:{}, Assert failed: {}{}\n", fmt::FG::Red, file, line, condition, fmt::FG::Reset);
#if COMPILER == MSVC && !defined LSTD_NO_CRT
    __debugbreak();
#else
    os_exit_program(-1);
#endif
}

#define CONSOLE_BUFFER_SIZE 1_KiB
#define MAX_CONSOLE_LINES 500

byte g_ConsoleWriterBuffer[CONSOLE_BUFFER_SIZE];

io::Console_Writer::Console_Writer() {
    Buffer = g_ConsoleWriterBuffer;
    Current = Buffer;
    Available = CONSOLE_BUFFER_SIZE;

    write_function = console_writer_write;
    flush_function = console_writer_flush;
}

static void console_writer_write_wrapper(io::Console_Writer *writer, const Memory_View &writeData) {
    if (writeData.ByteLength > writer->Available) {
        writer->flush();
    }

    copy_memory(writer->Current, writeData.Data, writeData.ByteLength);
    writer->Current += writeData.ByteLength;
    writer->Available -= writeData.ByteLength;
}

void io::console_writer_write(void *data, const Memory_View &writeData) {
    auto *writer = (Console_Writer *) data;

    if (!writer->Mutex) {
        writer->Mutex = new thread::Recursive_Mutex;
    }

    if (writer->LockMutex) {
        thread::Scoped_Lock<thread::Recursive_Mutex> _(*writer->Mutex);
        console_writer_write_wrapper(writer, writeData);
    } else {
        console_writer_write_wrapper(writer, writeData);
    }
}

static bool g_ConsoleAllocated = false;
static HANDLE g_CoutHandle = null;

static void allocate_console() {
    if (!g_ConsoleAllocated) {
        g_ConsoleAllocated = true;

        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();

            // set the screen buffer to be big enough to let us scroll text
            CONSOLE_SCREEN_BUFFER_INFO cInfo;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cInfo);
            cInfo.dwSize.Y = MAX_CONSOLE_LINES;
            SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), cInfo.dwSize);
        }
    }
}

static void console_writer_flush_wrapper(io::Console_Writer *writer) {
    if (!g_CoutHandle) {
        allocate_console();

        g_CoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (!SetConsoleOutputCP(CP_UTF8)) {
            string_view warning =
                ">>> Warning, couldn't set console code page to UTF-8. Some characters might be messed up.\n";

            DWORD ignored;
            WriteFile(g_CoutHandle, warning.Data, (DWORD) warning.ByteLength, &ignored, null);
        }

        // Enable colors with escape sequences
        DWORD dw = 0;
        GetConsoleMode(g_CoutHandle, &dw);
        SetConsoleMode(g_CoutHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    DWORD ignored;
    WriteFile(g_CoutHandle, writer->Buffer, (DWORD)(CONSOLE_BUFFER_SIZE - writer->Available), &ignored, null);

    writer->Current = writer->Buffer;
    writer->Available = CONSOLE_BUFFER_SIZE;
}

void io::console_writer_flush(void *data) {
    auto *writer = (Console_Writer *) data;

    if (!writer->Mutex) {
        writer->Mutex = new thread::Recursive_Mutex;
    }

    if (writer->LockMutex) {
        thread::Scoped_Lock<thread::Recursive_Mutex> _(*writer->Mutex);
        console_writer_flush_wrapper(writer);
    } else {
        console_writer_flush_wrapper(writer);
    }
}

byte g_ConsoleReaderBuffer[CONSOLE_BUFFER_SIZE];

io::Console_Reader::Console_Reader() {
    Buffer = g_ConsoleReaderBuffer;
    Current = Buffer;

    request_byte_function = console_reader_request_byte;
}

static HANDLE g_CinHandle = null;

static byte console_reader_wrapper(io::Console_Reader *reader) {
    if (!g_CinHandle) {
        allocate_console();
        g_CinHandle = GetStdHandle(STD_INPUT_HANDLE);
    }
    assert(reader->Available == 0);  // Sanity

    DWORD read;
    ReadFile(g_CinHandle, (char *) reader->Buffer, (DWORD) CONSOLE_BUFFER_SIZE, &read, null);

    reader->Current = reader->Buffer;
    reader->Available = read;

    return (read == 0) ? io::eof : (*reader->Current);
}

byte io::console_reader_request_byte(void *data) {
    auto *reader = (io::Console_Reader *) data;

    if (!reader->Mutex) {
        reader->Mutex = new thread::Mutex;
    }

    if (reader->LockMutex) {
        thread::Scoped_Lock<thread::Mutex> _(*reader->Mutex);
        return console_reader_wrapper(reader);
    } else {
        return console_reader_wrapper(reader);
    }
}

static LARGE_INTEGER g_PerformanceFrequency = {0};

s64 os_get_wallclock() {
    if (g_PerformanceFrequency.QuadPart == 0) {
        if (!QueryPerformanceFrequency(&g_PerformanceFrequency)) {
            return 0;
        }
    }
    LARGE_INTEGER time;
    if (!QueryPerformanceCounter(&time)) {
        return 0;
    }
    return time.QuadPart;
}

f64 os_get_elapsed_in_seconds(s64 begin, s64 end) { return (f64)(end - begin) / (f64) g_PerformanceFrequency.QuadPart; }
f64 os_get_wallclock_in_seconds() { return (f64) os_get_wallclock() / (f64) g_PerformanceFrequency.QuadPart; }

// All windows terminals support colors
bool fmt::internal::does_terminal_support_color() { return true; }

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS