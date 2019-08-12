#include "lstd/common.h"

#if OS == WINDOWS

#include "lstd/io.h"
#include "lstd/io/fmt.h"
#include "lstd/memory/dynamic_library.h"
#include "lstd/os.h"

#undef MAC
#undef _MAC
#include <Windows.h>

LSTD_BEGIN_NAMESPACE

bool dynamic_library::load(string name) {
    auto *buffer = new wchar_t[name.Length];
    defer(delete buffer);

    utf8_to_utf16(name.Data, name.Length, buffer);
    Handle = (void *) LoadLibraryW(buffer);
    return Handle;
}

void *dynamic_library::get_symbol(string name) {
    auto *buffer = new char[name.ByteLength + 1];
    defer(delete buffer);

    copy_memory(buffer, name.Data, name.ByteLength);
    buffer[name.ByteLength] = '\0';
    return (void *) GetProcAddress((HMODULE) Handle, (LPCSTR) buffer);
}

void dynamic_library::close() {
    if (Handle) {
        FreeLibrary((HMODULE) Handle);
        Handle = null;
    }
}

struct win32_state {
    static constexpr size_t CONSOLE_BUFFER_SIZE = 1_KiB;

    char CinBuffer[CONSOLE_BUFFER_SIZE]{};
    char CoutBuffer[CONSOLE_BUFFER_SIZE]{};
    char CerrBuffer[CONSOLE_BUFFER_SIZE]{};
    HANDLE CinHandle = null, CoutHandle = null, CerrHandle = null;
    LARGE_INTEGER PerformanceFrequency;
    string ModuleName;

    win32_state() {
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();

            // set the screen buffer to be big enough to let us scroll text
            CONSOLE_SCREEN_BUFFER_INFO cInfo;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cInfo);
            cInfo.dwSize.Y = 500;
            SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), cInfo.dwSize);
        }

        CinHandle = GetStdHandle(STD_INPUT_HANDLE);
        CoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CerrHandle = GetStdHandle(STD_ERROR_HANDLE);

        if (!SetConsoleOutputCP(CP_UTF8)) {
            string warning =
                ">>> Warning, couldn't set console code page to UTF-8. Some characters might be messed up.\n";

            DWORD ignored;
            WriteFile(CerrHandle, warning.Data, (DWORD) warning.ByteLength, &ignored, null);
        }

        // Enable ANSII escape sequences
        DWORD dw = 0;
        GetConsoleMode(CoutHandle, &dw);
        SetConsoleMode(CoutHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

        GetConsoleMode(CerrHandle, &dw);
        SetConsoleMode(CerrHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

        QueryPerformanceFrequency(&PerformanceFrequency);

        // Get the exe name
        wchar_t stackBuffer[MAX_PATH];
        size_t written, reserved = MAX_PATH;
        wchar_t *buffer = stackBuffer;

        while (true) {
            written = GetModuleFileNameW(null, buffer, (DWORD) reserved);
            if (written == reserved) {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    reserved *= 2;
                    if (buffer != stackBuffer) delete[] buffer;
                    buffer = new wchar_t[reserved];
                    continue;
                }
            }
            break;
        }
        if (buffer != stackBuffer) delete[] buffer;

        ModuleName.reserve(reserved * 2);
        utf16_to_utf8(buffer, const_cast<char *>(ModuleName.Data), &ModuleName.ByteLength);
        ModuleName.Length = utf8_strlen(ModuleName.Data, ModuleName.ByteLength);
    }
};
static win32_state STATE;

char io::console_reader_request_byte(io::reader *r) {
    auto *cr = (io::console_reader *) r;

    // @Thread
    // if (cr->LockMutex) {...}

    if (!cr->Buffer) cr->Buffer = cr->Current = STATE.CinBuffer;
    assert(cr->Available == 0);

    DWORD read;
    ReadFile(STATE.CinHandle, const_cast<char *>(cr->Buffer), (DWORD) STATE.CONSOLE_BUFFER_SIZE, &read, null);

    cr->Current = cr->Buffer;
    cr->Available = read;

    return (read == 0) ? io::eof : (*cr->Current);
}

void io::console_writer_write(io::writer *w, const char *data, size_t count) {
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

    if (!cw->Buffer) {
        if (cw->OutputType == io::console_writer::COUT) {
            cw->Buffer = cw->Current = STATE.CoutBuffer;
        } else {
            cw->Buffer = cw->Current = STATE.CerrBuffer;
        }

        cw->BufferSize = cw->Available = STATE.CONSOLE_BUFFER_SIZE;
    }

    HANDLE target = cw->OutputType == io::console_writer::COUT ? STATE.CoutHandle : STATE.CerrHandle;

    DWORD ignored;
    WriteFile(target, cw->Buffer, (DWORD)(cw->BufferSize - cw->Available), &ignored, null);

    cw->Current = cw->Buffer;
    cw->Available = STATE.CONSOLE_BUFFER_SIZE;
}

// This workaround is needed in order to prevent circular inclusion of context.h
namespace internal {
io::writer *g_ConsoleLog = &io::cout;
}

void *os_alloc(size_t size) { return HeapAlloc(GetProcessHeap(), 0, size); }

void os_free(void *ptr) { HeapFree(GetProcessHeap(), 0, ptr); }

void os_exit(s32 exitCode) { ExitProcess(exitCode); }

time_t os_get_time() {
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
#if BITS == 32
    return count.LowPart;
#else
    return count.QuadPart;
#endif
}

f64 os_time_to_seconds(time_t time) { return (f64) time / STATE.PerformanceFrequency.QuadPart; }

string os_get_exe_name() { return STATE.ModuleName; }

LSTD_END_NAMESPACE

#endif