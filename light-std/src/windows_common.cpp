#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/io.h"
#include "lstd/io/fmt.h"
#include "lstd/memory/dynamic_library.h"
#include "lstd/os.h"

#undef MAC
#undef _MAC
#include <Objbase.h>
#include <Windows.h>
#include <shellapi.h>  // @DependencyCleanup: CommandLineToArgvW

LSTD_BEGIN_NAMESPACE

#if COMPILER == MSVC
void win32_common_init();

extern void win32_window_init();
extern void win32_destroy_windows();
extern void win32_monitor_init();
extern void win32_crash_handler_init();

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
    win32_crash_handler_init();
    return 0;
}

s32 uninitialize_win32_state() {
    win32_destroy_windows();  // Needs to happend before the global WindowsList variable gets uninitialized
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

#pragma const_seg(".CRT$XPU")
__declspec(allocate(".CRT$XPU")) cb *g_StateAutoEnd = uninitialize_win32_state;
#pragma const_seg()

#else
#error @TODO: See how this works on other compilers!
#endif

bool dynamic_library::load(string name) {
    // @Bug value.Length is not enough (2 wide chars for one char)
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

static array<string> Argv;

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
    ModuleName.reserve(reserved * 2);  // @Bug reserved * 2 is not enough
    utf16_to_utf8(buffer, const_cast<char *>(ModuleName.Data), &ModuleName.ByteLength);
    ModuleName.Length = utf8_length(ModuleName.Data, ModuleName.ByteLength);

    // Get the arguments
    wchar_t **argv;
    int argc;

    // @Cleanup: Parse the arguments ourselves and use our temp allocator
    // and don't bother with cleaning up this functions memory.
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == null) {
        string warning =
            ">>> Warning, couldn't parse command line arguments, os_get_command_line_arguments() will return an empty "
            "array in all cases.\n";

        DWORD ignored;
        WriteFile(CerrHandle, warning.Data, (DWORD) warning.ByteLength, &ignored, null);
    } else {
        For(range(1, argc)) {
            auto *warg = argv[it];

            auto *arg = Argv.append();
            arg->reserve(c_string_length(warg) * 2);  // @Bug c_string_length * 2 is not enough
            utf16_to_utf8(warg, const_cast<char *>(arg->Data), &arg->ByteLength);
            arg->Length = utf8_length(arg->Data, arg->ByteLength);
        }

        LocalFree(argv);
    }
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

void *os_alloc(size_t size) { return GlobalAlloc(0, size); }

void os_free(void *ptr) { GlobalFree(ptr); }

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

bool os_get_env(string *out, string name, bool silent) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = new (Context.TemporaryAlloc) wchar_t[name.Length];
    utf8_to_utf16(name.Data, name.Length, name16);

    DWORD bufferSize = 65535;  // Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx

    auto *buffer = new (Context.TemporaryAlloc) wchar_t[bufferSize];
    auto r = GetEnvironmentVariableW(name16, buffer, bufferSize);

    if (r == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
        if (!silent) {
            PUSH_CONTEXT(Alloc, Context.TemporaryAlloc) {
                string warning = ">>> Warning, couldn't find environment variable with value \"";
                warning.append(name);
                warning.append("\".\n");

                DWORD ignored;
                WriteFile(CerrHandle, warning.Data, (DWORD) warning.ByteLength, &ignored, null);
            }
        }
        return false;
    }

    // 65535 may be the limit but let's not take risks
    if (r > bufferSize) {
        buffer = new (Context.TemporaryAlloc) wchar_t[r];
        GetEnvironmentVariableW(name16, buffer, r);
        bufferSize = r;

        // Possible to fail a second time ?
    }

    out->reserve(bufferSize * 2);  // @Bug bufferSize * 2 is not enough
    utf16_to_utf8(buffer, const_cast<char *>(out->Data), &out->ByteLength);
    out->Length = utf8_length(out->Data, out->ByteLength);

    return true;
}

void os_set_env(string name, string value) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = new (Context.TemporaryAlloc) wchar_t[name.Length];
    utf8_to_utf16(name.Data, name.Length, name16);

    // @Bug value.Length is not enough (2 wide chars for one char)
    auto *value16 = new (Context.TemporaryAlloc) wchar_t[value.Length];
    utf8_to_utf16(value.Data, value.Length, value16);

    if (value.Length > 32767) {
        // assert(false);
        // @Cleanup
        // The docs say windows doesn't allow that but we should test it.
    }

    if (!SetEnvironmentVariableW(name16, value16)) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()),
                                     "SetEnvironmentVariableW(LPCTSTR lpName, LPCTSTR lpValue)", __FILE__, __LINE__);
    }
}

void os_remove_env(string name) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = new (Context.TemporaryAlloc) wchar_t[name.Length];
    utf8_to_utf16(name.Data, name.Length, name16);

    if (!SetEnvironmentVariableW(name16, null)) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()),
                                     "SetEnvironmentVariableW(LPCTSTR lpName, LPCTSTR lpValue)", __FILE__, __LINE__);
    }
}

// Doesn't include the executable name.
array<string> os_get_command_line_arguments() { return Argv; }

u32 os_get_pid() { return (u32) GetCurrentProcessId(); }

guid new_guid() {
    GUID g;
    CoCreateGuid(&g);

    stack_array<char, 16> data = to_array((char) ((g.Data1 >> 24) & 0xFF), (char) ((g.Data1 >> 16) & 0xFF),
                                          (char) ((g.Data1 >> 8) & 0xFF), (char) ((g.Data1) & 0xff),

                                          (char) ((g.Data2 >> 8) & 0xFF), (char) ((g.Data2) & 0xff),

                                          (char) ((g.Data3 >> 8) & 0xFF), (char) ((g.Data3) & 0xFF),

                                          (char) g.Data4[0], (char) g.Data4[1], (char) g.Data4[2], (char) g.Data4[3],
                                          (char) g.Data4[4], (char) g.Data4[5], (char) g.Data4[6], (char) g.Data4[7]);
    return guid(array_view<char>(data.begin(), data.end()));
}

LSTD_END_NAMESPACE

#endif