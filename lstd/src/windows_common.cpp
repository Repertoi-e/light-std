#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/file/path.h"
#include "lstd/io.h"
#include "lstd/io/fmt.h"
#include "lstd/memory/dynamic_library.h"
#include "lstd/os.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

LSTD_BEGIN_NAMESPACE

static wchar_t *HelperClassName = null;

static HWND HelperWindowHandle;
static HDEVNOTIFY DeviceNotificationHandle;

static constexpr s64 CONSOLE_BUFFER_SIZE = 1_KiB;

static char CinBuffer[CONSOLE_BUFFER_SIZE]{};
static char CoutBuffer[CONSOLE_BUFFER_SIZE]{};
static char CerrBuffer[CONSOLE_BUFFER_SIZE]{};
static HANDLE CinHandle = null, CoutHandle = null, CerrHandle = null;
static thread::recursive_mutex CoutMutex;  // @Cleanup: recursive mutex
static thread::mutex CinMutex;

static LARGE_INTEGER PerformanceFrequency;
static string ModuleName;
static string WorkingDir;
static thread::mutex WorkingDirMutex;
static array<string> Argv;

static string ClipboardString;

// We must ensure that the context gets initialized before any global
// C++ constructors get called which may use the context.
s32 initialize_context() {
    Context = {};
    Context.TemporaryAlloc.Context = &Context.TemporaryAllocData;
    Context.ThreadID = thread::id((u64) GetCurrentThreadId());
    
#if defined DEBUG_MEMORY
    allocator::DEBUG_Mutex.init();
#endif
    
    CinMutex.init();
    CoutMutex.init();
    WorkingDirMutex.init();
    
    return 0;
}

void win32_common_init();

extern void win32_crash_handler_init();

// Needs to happen after global C++ constructors are initialized.
void initialize_win32_state() {
    win32_common_init();
    win32_crash_handler_init();
}

file_scope array<delegate<void()>> ExitFunctions;

void run_at_exit(const delegate<void()> &function) {
    WITH_CONTEXT_VAR(AllocOptions, Context.AllocOptions | LEAK) {
        ExitFunctions.append(function);
    }
}

// We supply this to the user if they are doing something very hacky..
void very_hacky_but_call_scheduled_exit_functions() {
    For(ExitFunctions) it();
}

// We supply this to the user if they are doing something very hacky..
array<delegate<void()>> *very_hacky_but_get_scheduled_exit_functions() {
    return &ExitFunctions;
}

// Needs to happen just before the global C++ destructors get called.
inline void call_exit_functions() {
    very_hacky_but_call_scheduled_exit_functions();
}

void uninitialize_win32_state() {
#if defined DEBUG_MEMORY
    release_temporary_allocator();
    
    // Now we check for memory leaks.
    // Yes, the OS claims back all the memory the program has allocated anyway, and we are not promoting C++ style RAII
    // which make even program termination slow, we are just providing this information to the user because they might
    // want to load/unload DLLs during the runtime of the application, and those DLLs might use all kinds of complex
    // cross-boundary memory stuff things, etc. This is useful for debugging crashes related to that.
    if (Context.CheckForLeaksAtTermination) {
        allocator::DEBUG_report_leaks();
    }
    
    // There's no better place to put this. Don't forget to call this for other operating systems!!!
    allocator::DEBUG_Mutex.release();
#endif
    
    CinMutex.release();
    CoutMutex.release();
    WorkingDirMutex.release();
}

//
// This trick makes all of the above requirements work on the MSVC compiler.
//
// How it works is described in this awesome article:
// https://www.codeguru.com/cpp/misc/misc/applicationcontrol/article.php/c6945/Running-Code-Before-and-After-Main.htm#page-2
#if COMPILER == MSVC
s32 c_init() {
    initialize_context();
    return 0;
}

s32 cpp_init() {
    initialize_win32_state();
    return 0;
}

s32 pre_termination() {
    call_exit_functions();
    uninitialize_win32_state();
    return 0;
}

#pragma push_macro("allocate")
#undef allocate

typedef s32 cb(void);
#pragma const_seg(".CRT$XIUSER")
__declspec(allocate(".CRT$XIUSER")) cb *g_CInit = c_init;
#pragma const_seg()

#pragma const_seg(".CRT$XCUSER")
__declspec(allocate(".CRT$XCUSER")) cb *g_CPPInit = cpp_init;
#pragma const_seg()

#pragma const_seg(".CRT$XPUSER")
__declspec(allocate(".CRT$XPUSER")) cb *g_PreTermination = pre_termination;
#pragma const_seg()

#pragma const_seg(".CRT$XTUSER")
__declspec(allocate(".CRT$XTUSER")) cb *g_Termination = NULL;
#pragma const_seg()

#pragma pop_macro("allocate")

#else
#error @TODO: See how this works on other compilers!
#endif

bool dynamic_library::load(const string &name) {
    // @Bug value.Length is not enough (2 wide chars for one char)
    auto *buffer = allocate_array(wchar_t, name.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(name.Data, name.Length, buffer);
    Handle = (void *) LoadLibraryW(buffer);
    return Handle;
}

void *dynamic_library::get_symbol(const string &name) {
    auto *buffer = allocate_array(char, name.ByteLength + 1, Context.TemporaryAlloc);
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

static void destroy_helper_window() {
    DestroyWindow(HelperWindowHandle);
}

static void register_helper_window_class() {
    GUID guid;
    WIN32_CHECKHR(CoCreateGuid(&guid));
    WIN32_CHECKHR(StringFromCLSID(guid, &HelperClassName));
    
    WNDCLASSEXW wc;
    zero_memory(&wc, sizeof(wc));
    {
        wc.cbSize = sizeof(wc);
        wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(null);
        wc.hCursor = LoadCursorW(null, IDC_ARROW);
        wc.lpszClassName = HelperClassName;
        
        // Load user-provided icon if available
        wc.hIcon = (HICON) LoadImageW(GetModuleHandleW(null), L"WINDOW ICON", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        if (!wc.hIcon) {
            // No user-provided icon found, load default icon
            wc.hIcon = (HICON) LoadImageW(null, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        }
    }
    
    if (!RegisterClassExW(&wc)) {
        fmt::print("(windows_common.cpp): Failed to register helper window class\n");
        assert(false);
    }
}

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
        string warning = ">>> Warning: Couldn't set console code page to UTF-8. Some characters might be messed up.\n";
        
        DWORD ignored;
        WriteFile(CerrHandle, warning.Data, (DWORD) warning.ByteLength, &ignored, null);
    }
    
    // Enable ANSI escape sequences
    DWORD dw = 0;
    GetConsoleMode(CoutHandle, &dw);
    SetConsoleMode(CoutHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    
    GetConsoleMode(CerrHandle, &dw);
    SetConsoleMode(CerrHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    
    QueryPerformanceFrequency(&PerformanceFrequency);
    
    // Get the module name
    wchar_t *buffer = allocate_array(wchar_t, MAX_PATH, Context.TemporaryAlloc);
    s64 reserved = MAX_PATH;
    
    while (true) {
        s64 written = GetModuleFileNameW((HMODULE) &__ImageBase, buffer, (DWORD) reserved);
        if (written == reserved) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                reserved *= 2;
                buffer = allocate_array(wchar_t, reserved, Context.TemporaryAlloc);
                continue;
            }
        }
        break;
    }
    
    WITH_CONTEXT_VAR(AllocOptions, Context.AllocOptions | LEAK) {
        ModuleName.reserve(reserved * 2);  // @Bug reserved * 2 is not enough
    }
    
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
            ">>> Warning: Couldn't parse command line arguments, os_get_command_line_arguments() will return an empty "
            "array in all cases.\n";
        
        DWORD ignored;
        WriteFile(CerrHandle, warning.Data, (DWORD) warning.ByteLength, &ignored, null);
    } else {
        WITH_CONTEXT_VAR(AllocOptions, Context.AllocOptions | LEAK) {
            Argv.reserve(argc - 1);
        }
        
        For(range(1, argc)) {
            auto *warg = argv[it];
            
            auto *arg = Argv.append();
            WITH_CONTEXT_VAR(AllocOptions, Context.AllocOptions | LEAK) {
                arg->reserve(c_string_length(warg) * 2);  // @Bug c_string_length * 2 is not enough
            }
            utf16_to_utf8(warg, const_cast<char *>(arg->Data), &arg->ByteLength);
            arg->Length = utf8_length(arg->Data, arg->ByteLength);
        }
        
        LocalFree(argv);
    }
    
    // Create helper window
    register_helper_window_class();
    
    {
        MSG msg;
        
        HelperWindowHandle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, HelperClassName, L"LSTD Message Window", WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 1, 1, null, null, GetModuleHandleW(null), null);
        if (!HelperWindowHandle) {
            fmt::print("(windows_monitor.cpp): Failed to create helper window\n");
            assert(false);
        }
        
        ShowWindow(HelperWindowHandle, SW_HIDE);
        
        // Register for HID device notifications
        {
            DEV_BROADCAST_DEVICEINTERFACE_W dbi;
            zero_memory(&dbi, sizeof(dbi));
            {
                dbi.dbcc_size = sizeof(dbi);
                dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
                dbi.dbcc_classguid = GUID_DEVINTERFACE_HID;
            }
            DeviceNotificationHandle = RegisterDeviceNotificationW(HelperWindowHandle, (DEV_BROADCAST_HDR *) &dbi,
                                                                   DEVICE_NOTIFY_WINDOW_HANDLE);
        }
        
        while (PeekMessageW(&msg, HelperWindowHandle, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    
    run_at_exit(destroy_helper_window);
}

char io::console_reader_request_byte(io::reader *r) {
    /*
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
    */
    return 0;
}

void io::console_writer_write(io::writer *w, const char *data, s64 count) {
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
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "HeapReAlloc", __FILE__, __LINE__);
    }
    return null;
}

s64 os_get_block_size(void *ptr) {
    s64 result = HeapSize(GetProcessHeap(), 0, ptr);
    if (result == -1) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "HeapSize", __FILE__, __LINE__);
        return 0;
    }
    return result;
}

#define CREATE_MAPPING_CHECKED(handleName, call, returnOnFail)                                                      \
HANDLE handleName = call;                                                                                       \
if (!handleName) {                                                                                              \
string extendedCallSite = fmt::sprint("{}\n        (the name was: {!YELLOW}\"{}\"{!GRAY})\n", #call, name); \
defer(extendedCallSite.release());                                                                          \
windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), extendedCallSite, __FILE__, __LINE__);     \
return returnOnFail;                                                                                        \
}

void os_write_shared_block(const string &name, void *data, s64 size) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = allocate_array(wchar_t, name.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(name.Data, name.Length, name16);
    
    CREATE_MAPPING_CHECKED(h,
                           CreateFileMappingW(INVALID_HANDLE_VALUE, null, PAGE_READWRITE, 0, (DWORD) size, name16), );
    defer(CloseHandle(h));
    
    void *result = MapViewOfFile(h, FILE_MAP_WRITE, 0, 0, size);
    if (!result) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "MapViewOfFile", __FILE__, __LINE__);
        return;
    }
    copy_memory(result, data, size);
    UnmapViewOfFile(result);
}

void os_read_shared_block(const string &name, void *out, s64 size) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = allocate_array(wchar_t, name.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(name.Data, name.Length, name16);
    
    CREATE_MAPPING_CHECKED(h, OpenFileMappingW(FILE_MAP_READ, false, name16), );
    defer(CloseHandle(h));
    
    void *result = MapViewOfFile(h, FILE_MAP_READ, 0, 0, size);
    if (!result) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "MapViewOfFile", __FILE__, __LINE__);
        return;
    }
    
    copy_memory(out, result, size);
    UnmapViewOfFile(result);
}

void os_free_block(void *ptr) {
    WIN32_CHECKBOOL(HeapFree(GetProcessHeap(), 0, ptr));
}

void os_exit(s32 exitCode) {
    call_exit_functions();
    uninitialize_win32_state();
    ExitProcess(exitCode);
}

time_t os_get_time() {
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return count.QuadPart;
}

f64 os_time_to_seconds(time_t time) { return (f64) time / PerformanceFrequency.QuadPart; }

string os_get_current_module() { return ModuleName; }

string os_get_working_dir() {
    thread::scoped_lock _(&WorkingDirMutex);
    
    DWORD required = GetCurrentDirectoryW(0, null);
    auto *dir16 = allocate_array(wchar_t, required + 1, Context.TemporaryAlloc);
    
    if (!GetCurrentDirectoryW(required + 1, dir16)) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "GetCurrentDirectoryW", __FILE__, __LINE__);
        return "";
    }
    
    WITH_CONTEXT_VAR(AllocOptions, Context.AllocOptions | LEAK) {
        WorkingDir.reserve(required * 2);  // @Bug required * 2 is not enough
    }
    
    utf16_to_utf8(dir16, const_cast<char *>(WorkingDir.Data), &WorkingDir.ByteLength);
    WorkingDir.Length = utf8_length(WorkingDir.Data, WorkingDir.ByteLength);
    return WorkingDir;
}

void os_set_working_dir(const string &dir) {
    file::path path(dir);
    assert(path.is_absolute());
    
    thread::scoped_lock _(&WorkingDirMutex);
    
    // @Bug
    auto *dir16 = allocate_array(wchar_t, dir.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(dir.Data, dir.Length, dir16);
    
    WIN32_CHECKBOOL(SetCurrentDirectoryW(dir16));
}

pair<bool, string> os_get_env(const string &name, bool silent) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = allocate_array(wchar_t, name.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(name.Data, name.Length, name16);
    
    DWORD bufferSize = 65535;  // Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx
    
    auto *buffer = allocate_array(wchar_t, bufferSize, Context.TemporaryAlloc);
    auto r = GetEnvironmentVariableW(name16, buffer, bufferSize);
    
    if (r == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
        if (!silent) {
            fmt::print(">>> Warning: Couldn't find environment variable with value \"{}\"\n", name);
        }
        return {false, ""};
    }
    
    // 65535 may be the limit but let's not take risks
    if (r > bufferSize) {
        buffer = allocate_array(wchar_t, r, Context.TemporaryAlloc);
        GetEnvironmentVariableW(name16, buffer, r);
        bufferSize = r;
        
        // Possible to fail a second time ?
    }
    
    string result;
    result.reserve(bufferSize * 2);  // @Bug bufferSize * 2 is not enough
    utf16_to_utf8(buffer, const_cast<char *>(result.Data), &result.ByteLength);
    result.Length = utf8_length(result.Data, result.ByteLength);
    
    return {true, result};
}

void os_set_env(const string &name, const string &value) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = allocate_array(wchar_t, name.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(name.Data, name.Length, name16);
    
    // @Bug value.Length is not enough (2 wide chars for one char)
    auto *value16 = allocate_array(wchar_t, value.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(value.Data, value.Length, value16);
    
    if (value.Length > 32767) {
        // assert(false);
        // @Cleanup
        // The docs say windows doesn't allow that but we should test it.
    }
    
    WIN32_CHECKBOOL(SetEnvironmentVariableW(name16, value16));
}

void os_remove_env(const string &name) {
    // @Bug name.Length is not enough (2 wide chars for one char)
    auto *name16 = allocate_array(wchar_t, name.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(name.Data, name.Length, name16);
    
    WIN32_CHECKBOOL(SetEnvironmentVariableW(name16, null));
}

string os_get_clipboard_content() {
    if (!OpenClipboard(HelperWindowHandle)) {
        fmt::print("(windows_monitor.cpp): Failed to open clipboard\n");
        return "";
    }
    defer(CloseClipboard());
    
    HANDLE object = GetClipboardData(CF_UNICODETEXT);
    if (!object) {
        fmt::print("(windows_monitor.cpp): Failed to convert clipboard to string\n");
        return "";
    }
    
    auto *buffer = (wchar_t *) GlobalLock(object);
    if (!buffer) {
        fmt::print("(windows_monitor.cpp): Failed to lock global handle\n");
        return "";
    }
    defer(GlobalUnlock(object));
    
    WITH_CONTEXT_VAR(AllocOptions, Context.AllocOptions | LEAK) {
        ClipboardString.reserve(c_string_length(buffer) * 2);  // @Bug c_string_length * 2 is not enough
    }
    
    utf16_to_utf8(buffer, const_cast<char *>(ClipboardString.Data), &ClipboardString.ByteLength);
    ClipboardString.Length = utf8_length(ClipboardString.Data, ClipboardString.ByteLength);
    
    return ClipboardString;
}

void os_set_clipboard_content(const string &content) {
    HANDLE object = GlobalAlloc(GMEM_MOVEABLE, content.Length * 2 * sizeof(wchar_t));
    if (!object) {
        fmt::print("(windows_monitor.cpp): Failed to open clipboard\n");
        return;
    }
    defer(GlobalFree(object));
    
    auto *buffer = (wchar_t *) GlobalLock(object);
    if (!buffer) {
        fmt::print("(windows_monitor.cpp): Failed to lock global handle\n");
        return;
    }
    
    utf8_to_utf16(content.Data, content.Length, buffer);
    GlobalUnlock(object);
    
    if (!OpenClipboard(HelperWindowHandle)) {
        fmt::print("(windows_monitor.cpp): Failed to open clipboard\n");
        return;
    }
    defer(CloseClipboard());
    
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, object);
    CloseClipboard();
}

// Doesn't include the exe name.
array<string> os_get_command_line_arguments() { return Argv; }

u32 os_get_pid() { return (u32) GetCurrentProcessId(); }

guid guid_new() {
    GUID g;
    CoCreateGuid(&g);
    
    stack_array<char, 16> data = to_stack_array((char) ((g.Data1 >> 24) & 0xFF), (char) ((g.Data1 >> 16) & 0xFF),
                                                (char) ((g.Data1 >> 8) & 0xFF), (char) ((g.Data1) & 0xff),
                                                
                                                (char) ((g.Data2 >> 8) & 0xFF), (char) ((g.Data2) & 0xff),
                                                
                                                (char) ((g.Data3 >> 8) & 0xFF), (char) ((g.Data3) & 0xFF),
                                                
                                                (char) g.Data4[0], (char) g.Data4[1], (char) g.Data4[2], (char) g.Data4[3],
                                                (char) g.Data4[4], (char) g.Data4[5], (char) g.Data4[6], (char) g.Data4[7]);
    return guid(data);
}

LSTD_END_NAMESPACE

#endif