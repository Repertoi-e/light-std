#include "lstd/common.hpp"

#if OS == WINDOWS

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "lstd/io.hpp"

LSTD_BEGIN_NAMESPACE

void *os_memory_alloc(void *context, size_t size, size_t *outsize) { return HeapAlloc(GetProcessHeap(), 0, size); }
void os_memory_free(void *context, void *ptr) { HeapFree(GetProcessHeap(), 0, ptr); }

void *os_allocator(allocator_mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, uptr_t) {
    switch (mode) {
        case allocator_mode::ALLOCATE: {
            void *data = os_memory_alloc(null, size, null);
            zero_memory(data, size);
            return data;
        }
        case allocator_mode::RESIZE:
            return HeapReAlloc(GetProcessHeap(), 0, oldMemory, size);
        case allocator_mode::FREE:
            os_memory_free(null, oldMemory);
            return null;
        case allocator_mode::FREE_ALL:
            assert(false);
            return null;
    }
    return null;
}

#if !defined LSTD_NO_CRT
void os_exit_program(s32 code) { exit(code); }
#endif

void os_assert_failed(const byte *file, s32 line, const byte *condition) {
    print("{}>>> {}:{}, Assert failed: {}{}\n", fmt::fg::Red, file, line, condition, fmt::fg::Reset);
#if COMPILER == MSVC && !defined LSTD_NO_CRT
    __debugbreak();
#else
    os_exit_program(-1);
#endif
}

#define CONSOLE_BUFFER_SIZE 1_KiB
#define MAX_CONSOLE_LINES 500

byte g_CoutBuffer[CONSOLE_BUFFER_SIZE];
byte g_CerrBuffer[CONSOLE_BUFFER_SIZE];

io::console_writer::console_writer() {
    Buffer = Current = g_CoutBuffer;
    Available = CONSOLE_BUFFER_SIZE;

    WriteFunction = console_writer_write;
    FlushFunction = console_writer_flush;
}

io::console_writer::console_writer(bool cerr) : console_writer() {
    _Err = cerr;
    if (cerr) Buffer = Current = g_CerrBuffer;
}

static void console_writer_write_wrapper(io::console_writer *writer, const memory_view &writeData) {
    if (writeData.ByteLength > writer->Available) {
        writer->flush();
    }

    copy_memory(writer->Current, writeData.Data, writeData.ByteLength);
    writer->Current += writeData.ByteLength;
    writer->Available -= writeData.ByteLength;
}

void io::console_writer_write(void *data, const memory_view &writeData) {
    auto *writer = (console_writer *) data;

    if (!writer->_Mutex) writer->_Mutex = new (MALLOC) thread::recursive_mutex;

    if (writer->LockMutex) {
        thread::scoped_lock<thread::recursive_mutex> _(*writer->_Mutex);
        console_writer_write_wrapper(writer, writeData);
    } else {
        console_writer_write_wrapper(writer, writeData);
    }
}

static bool g_ConsoleAllocated = false;
static HANDLE g_CoutHandle = null;
static HANDLE g_CerrHandle = null;

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

static void console_writer_flush_wrapper(io::console_writer *writer, bool cerr) {
    if (!g_CoutHandle) {
        allocate_console();

        g_CoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        g_CerrHandle = GetStdHandle(STD_ERROR_HANDLE);
        if (!SetConsoleOutputCP(CP_UTF8)) {
            string_view warning =
                ">>> Warning, couldn't set console code page to UTF-8. Some characters might be messed up.\n";

            DWORD ignored;
            WriteFile(g_CerrHandle, warning.Data, (DWORD) warning.ByteLength, &ignored, null);
        }

        // Enable colors with escape sequences
        DWORD dw = 0;
        GetConsoleMode(g_CoutHandle, &dw);
        SetConsoleMode(g_CoutHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

        GetConsoleMode(g_CerrHandle, &dw);
        SetConsoleMode(g_CerrHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    DWORD ignored;
    WriteFile(cerr ? g_CerrHandle : g_CoutHandle, writer->Buffer, (DWORD)(CONSOLE_BUFFER_SIZE - writer->Available),
              &ignored, null);

    writer->Current = writer->Buffer;
    writer->Available = CONSOLE_BUFFER_SIZE;
}

void io::console_writer_flush(void *data) {
    auto *writer = (console_writer *) data;

    if (!writer->_Mutex) writer->_Mutex = new (MALLOC) thread::recursive_mutex;

    if (writer->LockMutex) {
        thread::scoped_lock<thread::recursive_mutex> _(*writer->_Mutex);
        console_writer_flush_wrapper(writer, writer->_Err);
    } else {
        console_writer_flush_wrapper(writer, writer->_Err);
    }
}

byte g_CinBuffer[CONSOLE_BUFFER_SIZE];

io::console_reader::console_reader() {
    Buffer = g_CinBuffer;
    Current = Buffer;

    request_byte_function = console_reader_request_byte;
}

static HANDLE g_CinHandle = null;

static byte console_reader_wrapper(io::console_reader *reader) {
    if (!g_CinHandle) {
        allocate_console();
        g_CinHandle = GetStdHandle(STD_INPUT_HANDLE);
    }
    assert(reader->Available == 0);  // Sanity

    DWORD read;
    ReadFile(g_CinHandle, const_cast<byte *>(reader->Buffer), (DWORD) CONSOLE_BUFFER_SIZE, &read, null);

    reader->Current = reader->Buffer;
    reader->Available = read;

    return (read == 0) ? io::eof : (*reader->Current);
}

byte io::console_reader_request_byte(void *data) {
    auto *reader = (console_reader *) data;

    if (!reader->_Mutex) {
        reader->_Mutex = new (MALLOC) thread::mutex;
    }

    if (reader->LockMutex) {
        thread::scoped_lock<thread::mutex> _(*reader->_Mutex);
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