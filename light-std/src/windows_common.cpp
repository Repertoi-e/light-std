#include "lstd/common.h"

#if OS == WINDOWS

#include "lstd/io/console_reader.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void *os_alloc(size_t size) { return HeapAlloc(GetProcessHeap(), 0, size); }

void os_free(void *ptr) { HeapFree(GetProcessHeap(), 0, ptr); }

#define CONSOLE_BUFFER_SIZE 1_KiB

static bool g_ConsoleAllocated = false;
static byte g_CinBuffer[CONSOLE_BUFFER_SIZE]{};
static HANDLE g_CinHandle = null, g_CoutHandle = null, g_CerrHandle = null;

static void allocate_console() {
    if (!g_ConsoleAllocated) {
        g_ConsoleAllocated = true;

        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();

            // set the screen buffer to be big enough to let us scroll text
            CONSOLE_SCREEN_BUFFER_INFO cInfo;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cInfo);
            cInfo.dwSize.Y = 500;
            SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), cInfo.dwSize);
        }
    }
}

static byte console_reader_wrapper(io::console_reader *reader) {
    if (!g_CinHandle) {
        allocate_console();
        g_CinHandle = GetStdHandle(STD_INPUT_HANDLE);

        reader->Buffer = reader->Current = g_CinBuffer;
    }
    assert(reader->Available == 0);

    DWORD read;
    ReadFile(g_CinHandle, const_cast<byte *>(reader->Buffer), (DWORD) CONSOLE_BUFFER_SIZE, &read, null);

    reader->Current = reader->Buffer;
    reader->Available = read;

    return (read == 0) ? io::eof : (*reader->Current);
}

byte io::console_reader_request_byte(reader *data) {
    auto *reader = (console_reader *) data;

    // @Thread
    // if (reader->LockMutex) {...}
    return console_reader_wrapper(reader);
}

#endif