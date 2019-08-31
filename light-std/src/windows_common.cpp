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

#if COMPILER == MSVC
void win32_common_init();

extern void win32_window_init();
extern void win32_monitor_init();

// This trick ensures the context gets initialized before any C++ constructors
// get called which may use the context.
//
// How it works is described here:
// https://www.codeguru.com/cpp/misc/misc/applicationcontrol/article.php/c6945/Running-Code-Before-and-After-Main.htm#page-2
s32 initialize_context_and_global_state() {
    auto *unconstContext = const_cast<implicit_context *>(&Context);
    *unconstContext = {};
    unconstContext->TemporaryAlloc.Context = &unconstContext->TemporaryAllocData;
    unconstContext->ThreadID = thread::id((u64) GetCurrentThreadId());
    return 0;
}

s32 initialize_win32_state() {
    win32_common_init();
    win32_window_init();
    win32_monitor_init();
    return 0;
}

typedef s32 cb(void);
#pragma const_seg(".CRT$XIU")
__declspec(allocate(".CRT$XIU")) cb *g_ContextAutoStart = initialize_context_and_global_state;
#pragma const_seg()

// @Hack It should end in U (source: the link) but then it doesn't get called after all other C++ constructors...
#pragma const_seg(".CRT$XCZ")
__declspec(allocate(".CRT$XCZ")) cb *g_StateAutoStart = initialize_win32_state;
#pragma const_seg()

#else
#error @TODO: See how this works on other compilers!
#endif

bool dynamic_library::load(string name) {
    auto *buffer = new (Context.TemporaryAlloc) wchar_t[name.Length];
    utf8_to_utf16(name.Data, name.Length, buffer);
    Handle = (void *) LoadLibraryW(buffer);
    return Handle;
}

void *dynamic_library::get_symbol(string name) {
    auto *buffer = new (Context.TemporaryAlloc) char[name.ByteLength + 1];
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

static constexpr size_t CONSOLE_BUFFER_SIZE = 1_KiB;

static char CinBuffer[CONSOLE_BUFFER_SIZE]{};
static char CoutBuffer[CONSOLE_BUFFER_SIZE]{};
static char CerrBuffer[CONSOLE_BUFFER_SIZE]{};
static HANDLE CinHandle = null, CoutHandle = null, CerrHandle = null;
static thread::recursive_mutex CoutMutex;
static thread::mutex CinMutex;

static LARGE_INTEGER PerformanceFrequency;
static string ModuleName;

void win32_common_init() {
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
        string warning = ">>> Warning, couldn't set console code page to UTF-8. Some characters might be messed up.\n";

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
    wchar_t *buffer = new (Context.TemporaryAlloc) wchar_t[MAX_PATH];
    size_t reserved = MAX_PATH;

    while (true) {
        size_t written = GetModuleFileNameW(null, buffer, (DWORD) reserved);
        if (written == reserved) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                reserved *= 2;
                buffer = new (Context.TemporaryAlloc) wchar_t[reserved];
                continue;
            }
        }
        break;
    }
    ModuleName.reserve(reserved * 2);
    utf16_to_utf8(buffer, const_cast<char *>(ModuleName.Data), &ModuleName.ByteLength);
    ModuleName.Length = utf8_strlen(ModuleName.Data, ModuleName.ByteLength);
}

char io::console_reader_request_byte(io::reader *r) {
    auto *cr = (io::console_reader *) r;

    thread::mutex *mutex = null;
    if (cr->LockMutex) mutex = &CinMutex;
    thread::scoped_lock<thread::mutex> _(mutex);

    if (!cr->Buffer) cr->Buffer = cr->Current = CinBuffer;
    assert(cr->Available == 0);

    DWORD read;
    ReadFile(CinHandle, const_cast<char *>(cr->Buffer), (DWORD) CONSOLE_BUFFER_SIZE, &read, null);

    cr->Current = cr->Buffer;
    cr->Available = read;

    return (read == 0) ? io::eof : (*cr->Current);
}

void io::console_writer_write(io::writer *w, const char *data, size_t count) {
    auto *cw = (io::console_writer *) w;

    thread::recursive_mutex *mutex = null;
    if (cw->LockMutex) mutex = &CoutMutex;
    thread::scoped_lock<thread::recursive_mutex> _(mutex);

    if (count > cw->Available) {
        cw->flush();
    }

    copy_memory(cw->Current, data, count);
    cw->Current += count;
    cw->Available -= count;
}

void io::console_writer_flush(io::writer *w) {
    auto *cw = (io::console_writer *) w;

    thread::recursive_mutex *mutex = null;
    if (cw->LockMutex) mutex = &CoutMutex;
    thread::scoped_lock<thread::recursive_mutex> _(mutex);

    if (!cw->Buffer) {
        if (cw->OutputType == io::console_writer::COUT) {
            cw->Buffer = cw->Current = CoutBuffer;
        } else {
            cw->Buffer = cw->Current = CerrBuffer;
        }

        cw->BufferSize = cw->Available = CONSOLE_BUFFER_SIZE;
    }

    HANDLE target = cw->OutputType == io::console_writer::COUT ? CoutHandle : CerrHandle;

    DWORD ignored;
    WriteFile(target, cw->Buffer, (DWORD)(cw->BufferSize - cw->Available), &ignored, null);

    cw->Current = cw->Buffer;
    cw->Available = CONSOLE_BUFFER_SIZE;
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

f64 os_time_to_seconds(time_t time) { return (f64) time / PerformanceFrequency.QuadPart; }

string os_get_exe_name() { return ModuleName; }

LSTD_END_NAMESPACE

#endif