#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/file/path.h"
#include "lstd/io.h"
#include "lstd/io/fmt.h"
#include "lstd/memory/dynamic_library.h"
#include "lstd/os.h"

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
    Context = {};
    Context.TemporaryAlloc.Context = &Context.TemporaryAllocData;
    Context.ThreadID = thread::id((u64) GetCurrentThreadId());
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
    auto *buffer = new (Context.TemporaryAlloc) wchar_t[name.Length + 1];
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

static constexpr s64 CONSOLE_BUFFER_SIZE = 1_KiB;

static char CinBuffer[CONSOLE_BUFFER_SIZE]{};
static char CoutBuffer[CONSOLE_BUFFER_SIZE]{};
static char CerrBuffer[CONSOLE_BUFFER_SIZE]{};
static HANDLE CinHandle = null, CoutHandle = null, CerrHandle = null;
static thread::mutex CoutMutex;
static thread::mutex CinMutex;

static LARGE_INTEGER PerformanceFrequency;
static string ModuleName;
static string WorkingDir;
static thread::mutex WorkingDirMutex;
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
    s64 reserved = MAX_PATH;

    while (true) {
        s64 written = GetModuleFileNameW(null, buffer, (DWORD) reserved);
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

    os_get_working_dir();  // @Hack Put somethin in _WorkingDir_ to ensure the proper allocator

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

void io::console_writer_write(io::writer *w, const char *data, s64 count) {
    auto *cw = (io::console_writer *) w;

    if (count > cw->Available) {
        cw->flush();
    }

    thread::mutex *mutex = null;
    if (cw->LockMutex) mutex = &CoutMutex;

    thread::scoped_lock<thread::mutex> _(mutex);

    copy_memory(cw->Current, data, count);

    cw->Current += count;
    cw->Available -= count;
}

void io::console_writer_flush(io::writer *w) {
    auto *cw = (io::console_writer *) w;

    thread::mutex *mutex = null;
    if (cw->LockMutex) mutex = &CoutMutex;

    thread::scoped_lock<thread::mutex> _(mutex);

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

void *os_allocate_block(s64 size) {
    assert(size < MAX_ALLOCATION_REQUEST);
    return HeapAlloc(GetProcessHeap(), 0, size);
}

// Tests whether the allocation contraction is possible
static bool is_contraction_possible(s64 oldSize) {
    // Check if object allocated on low fragmentation heap.
    // The LFH can only allocate blocks up to 16KB in size.
    if (oldSize <= 0x4000) {
        LONG heapType = -1;
        if (!HeapQueryInformation(GetProcessHeap(), HeapCompatibilityInformation, &heapType, sizeof(heapType), null)) {
            return false;
        }
        return heapType != 2;
    }

    // Contraction possible for objects not on the LFH
    return true;
}

static void *try_heap_realloc(void *ptr, s64 newSize, bool *reportError) {
    void *result = null;
    __try {
        result = HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY | HEAP_GENERATE_EXCEPTIONS, ptr, newSize);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // We specify HEAP_REALLOC_IN_PLACE_ONLY, so STATUS_NO_MEMORY is a valid error.
        // We don't need to report it.
        *reportError = GetExceptionCode() != STATUS_NO_MEMORY;
    }
    return result;
}

void *os_resize_block(void *ptr, s64 newSize) {
    assert(ptr);
    assert(newSize < MAX_ALLOCATION_REQUEST);

    s64 oldSize = os_get_block_size(ptr);
    if (newSize == 0) newSize = 1;

    bool reportError = false;
    void *result = try_heap_realloc(ptr, newSize, &reportError);

    if (result) return result;

    // If a failure to contract was caused by platform limitations, just return the original block
    if (newSize < oldSize && !is_contraction_possible(oldSize)) return ptr;

    if (reportError) {
        windows_report_hresult_error(
            HRESULT_FROM_WIN32(GetLastError()),
            "HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY | HEAP_GENERATE_EXCEPTIONS, ptr, newSize)",
            __FILE__, __LINE__);
    }
    return null;
}

s64 os_get_block_size(void *ptr) {
    s64 result = HeapSize(GetProcessHeap(), 0, ptr);
    if (result == -1) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "HeapSize(GetProcessHeap(), 0, ptr)", __FILE__,
                                     __LINE__);
        return 0;
    }
    return result;
}

#define CREATE_MAPPING_CHECKED(handleName, call, returnOnFail)                                                  \
    HANDLE handleName = call;                                                                                   \
    if (!handleName) {                                                                                          \
        string extendedCallSite;                                                                                \
        fmt::sprint(&extendedCallSite, "{}\n        (the name was: {!YELLOW}\"{}\"{!GRAY})\n", #call, name);    \
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), extendedCallSite, __FILE__, __LINE__); \
        return returnOnFail;                                                                                    \
    }

void os_write_shared_block(string name, void *data, s64 size) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = new (Context.TemporaryAlloc) wchar_t[name.Length + 1];
    utf8_to_utf16(name.Data, name.Length, name16);

    CREATE_MAPPING_CHECKED(h,
                           CreateFileMappingW(INVALID_HANDLE_VALUE, null, PAGE_READWRITE, 0, (DWORD) size, name16), );
    defer(CloseHandle(h));

    void *result = MapViewOfFile(h, FILE_MAP_WRITE, 0, 0, size);
    if (!result) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "MapViewOfFile(h, FILE_MAP_WRITE, 0, 0, size)",
                                     __FILE__, __LINE__);
        return;
    }
    copy_memory(result, data, size);
    UnmapViewOfFile(result);
}

void os_read_shared_block(string name, void *out, s64 size) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = new (Context.TemporaryAlloc) wchar_t[name.Length + 1];
    utf8_to_utf16(name.Data, name.Length, name16);

    CREATE_MAPPING_CHECKED(h, OpenFileMappingW(FILE_MAP_READ, false, name16), );
    defer(CloseHandle(h));

    void *result = MapViewOfFile(h, FILE_MAP_READ, 0, 0, size);
    if (!result) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "MapViewOfFile(h, FILE_MAP_READ, 0, 0, size)",
                                     __FILE__, __LINE__);
        return;
    }

    copy_memory(out, result, size);
    UnmapViewOfFile(result);
}

void os_free_block(void *ptr) {
    if (!HeapFree(GetProcessHeap(), 0, ptr)) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "HeapFree(GetProcessHeap(), 0, ptr)", __FILE__,
                                     __LINE__);
    }
}

void os_exit(s32 exitCode) { ExitProcess(exitCode); }

time_t os_get_time() {
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return count.QuadPart;
}

f64 os_time_to_seconds(time_t time) { return (f64) time / PerformanceFrequency.QuadPart; }

string os_get_exe_name() { return ModuleName; }

string os_get_working_dir() {
    thread::scoped_lock _(&WorkingDirMutex);

    DWORD required = GetCurrentDirectoryW(0, null);
    auto *dir16 = new (Context.TemporaryAlloc) wchar_t[required + 1];

    if (!GetCurrentDirectoryW(required + 1, dir16)) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "GetCurrentDirectoryW(required, dir16)",
                                     __FILE__, __LINE__);
        return "";
    }
    WorkingDir.reserve(required * 2);  // @Bug required * 2 is not enough
    utf16_to_utf8(dir16, const_cast<char *>(WorkingDir.Data), &WorkingDir.ByteLength);
    WorkingDir.Length = utf8_length(WorkingDir.Data, WorkingDir.ByteLength);
    return WorkingDir;
}

void os_set_working_dir(string dir) {
    file::path path(dir);
    assert(path.is_absolute());

    thread::scoped_lock _(&WorkingDirMutex);

    auto *dir16 = new (Context.TemporaryAlloc) wchar_t[dir.Length + 1];
    utf8_to_utf16(dir.Data, dir.Length, dir16);

    if (!SetCurrentDirectoryW(dir16)) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "GetCurrentDirectoryW(required, dir16)",
                                     __FILE__, __LINE__);
    }
}

bool os_get_env(string *out, string name, bool silent) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = new (Context.TemporaryAlloc) wchar_t[name.Length + 1];
    utf8_to_utf16(name.Data, name.Length, name16);

    DWORD bufferSize = 65535;  // Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx

    auto *buffer = new (Context.TemporaryAlloc) wchar_t[bufferSize];
    auto r = GetEnvironmentVariableW(name16, buffer, bufferSize);

    if (r == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
        if (!silent) {
            WITH_ALLOC(Context.TemporaryAlloc) {
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
    auto *name16 = new (Context.TemporaryAlloc) wchar_t[name.Length + 1];
    utf8_to_utf16(name.Data, name.Length, name16);

    // @Bug value.Length is not enough (2 wide chars for one char)
    auto *value16 = new (Context.TemporaryAlloc) wchar_t[value.Length + 1];
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
    auto *name16 = new (Context.TemporaryAlloc) wchar_t[name.Length + 1];
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