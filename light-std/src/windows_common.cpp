#include "lstd/common.h"

#if OS == WINDOWS

#include "lstd/io.h"
#include "lstd/io/fmt.h"

#include <Windows.h>

LSTD_BEGIN_NAMESPACE

void *os_alloc(size_t size) { return HeapAlloc(GetProcessHeap(), 0, size); }

void os_free(void *ptr) { HeapFree(GetProcessHeap(), 0, ptr); }

#define CONSOLE_BUFFER_SIZE 1_KiB

static bool g_ConsoleAllocated = false;
static byte g_CinBuffer[CONSOLE_BUFFER_SIZE]{};
static byte g_CoutBuffer[CONSOLE_BUFFER_SIZE]{};
static byte g_CerrBuffer[CONSOLE_BUFFER_SIZE]{};
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

byte io::console_reader_request_byte(io::reader *r) {
    auto *cr = (io::console_reader *) r;

    // @Thread
    // if (cr->LockMutex) {...}

    if (!g_CinHandle) {
        allocate_console();

        g_CinHandle = GetStdHandle(STD_INPUT_HANDLE);
        cr->Buffer = cr->Current = g_CinBuffer;
    }
    assert(cr->Available == 0);

    DWORD read;
    ReadFile(g_CinHandle, const_cast<byte *>(cr->Buffer), (DWORD) CONSOLE_BUFFER_SIZE, &read, null);

    cr->Current = cr->Buffer;
    cr->Available = read;

    return (read == 0) ? io::eof : (*cr->Current);
}

void io::console_writer_write(io::writer *w, const byte *data, size_t count) {
    auto *cw = (io::console_writer *) w;

    // @Thread
    // if (cw->LockMutex) { ... }

    if (count > cw->Available) {
        cw->flush();
    }

    copy_memory(cw->Current, data, count);
    cw->Current += count;
    cw->Available -= count;
}

void io::console_writer_flush(io::writer *w) {
    auto *cw = (io::console_writer *) w;

    if (!g_CoutHandle || !g_CerrHandle) {
        allocate_console();

        g_CoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        g_CerrHandle = GetStdHandle(STD_ERROR_HANDLE);

        if (cw->OutputType == io::console_writer::COUT) {
            cw->Buffer = cw->Current = g_CoutBuffer;
        } else {
            cw->Buffer = cw->Current = g_CerrBuffer;
        }

        cw->BufferSize = cw->Available = CONSOLE_BUFFER_SIZE;

        if (!SetConsoleOutputCP(CP_UTF8)) {
            string warning =
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

    HANDLE target = cw->OutputType == io::console_writer::COUT ? g_CoutHandle : g_CerrHandle;

    DWORD ignored;
    WriteFile(target, cw->Buffer, (DWORD)(cw->BufferSize - cw->Available), &ignored, null);

    cw->Current = cw->Buffer;
    cw->Available = CONSOLE_BUFFER_SIZE;
}

// This workaround is needed in order to prevent circular inclusion of context.h
namespace internal {
io::writer *g_ConsoleLog = &io::cout;
}

LSTD_END_NAMESPACE

#endif