module;

#include "lstd/platform/windows.h"  // Declarations of Win32 functions
#include "platform_uninit.h"

//
// Simple wrapper around dynamic libraries and getting addresses of procedures.
//

export module lstd.os.win32.common;

import lstd.os.win32.memory;

import lstd.fmt;
import lstd.path;
import lstd.writer;
import lstd.thread;
import lstd.console;

extern "C" IMAGE_DOS_HEADER __ImageBase;
#define MODULE_HANDLE ((HMODULE) &__ImageBase)

LSTD_BEGIN_NAMESPACE

//
// @Platform @Cleanup @TODO: These declarations shouldn't be specific to win32.
// Perhaps put them in a general module?
// This also applies to the _path_ and _thread_ modules.
//

export {
    // Exits the application with the given exit code.
    // Also runs all callbacks registered with exit_schedule().
    void exit(s32 exitCode = 0);

    // Produces an abnormal program termination.
    // Doesn't call any callbacks registered with exit_schedule().
    void abort();

    // Schedule a function to run when this module exists - before any global C++ destructors are called
    // and before the CRT (if we are linking against it) uninitializes.
    //
    // Also runs when calling exit(exitCode).
    //
    // Note: We don't try to be as general as possible. exit_schedule() is merely a utility that might be useful
    // to ensure e.g. files are flushed or handles closed. (Don't use this for freeing memory, the OS claims it back anyway!!).
    void exit_schedule(const delegate<void()> &function);

    // Runs all scheduled exit functions.
    // This is exported if you want to do something very weird and hacky.
    void exit_call_scheduled_functions();

    // Returns a pointer so you can modify the array of scheduled functions.
    // This is exported if you want to do something very weird and hacky.
    array<delegate<void()>> *exit_get_scheduled_functions();

    struct os_read_file_result {
        string Content;
        bool Success;
    };

    enum class file_write_mode {
        Append = 0,

        // If the file is 50 bytes and you write 20,
        // "Overwrite" keeps those 30 bytes at the end
        // while "Overwrite_Entire" deletes them.
        Overwrite,
        Overwrite_Entire,
    };

    // Reads entire file into memory (no async variant available at the moment).
    [[nodiscard("Leak")]] os_read_file_result os_read_entire_file(string path);

    // Write _contents_ to a file.
    // _mode_ determines if the content should be appended or overwritten. See _file_write_mode_ above.
    //
    // Returns true on success.
    bool os_write_to_file(string path, string contents, file_write_mode mode);

    // Returns a time stamp that can be used for time-interval measurements
    time_t os_get_time();

    // Converts a time stamp acquired by os_get_time() to seconds
    f64 os_time_to_seconds(time_t time);

    //
    // Note: The functions above don't have the "os_" prefix because they are not really doing stuff with the OS.
    // The functions below have the "os_" prefix and can be easily queried with autocomplete.
    //

    // Returns the path of the current module (executable or dynamic library).
    // Full dir + name.
    //
    // Note: Don't free the result of this function.
    string os_get_current_module();

    // Returns the current directory of the current process.
    // [Windows] The docs say that SetCurrentDirectory/GetCurrentDirectory
    //           are not thread-safe but we use a lock so ours are.
    // Note: Don't free the result of this function.
    string os_get_working_dir();

    // Sets the current directory of the current process.
    // [Windows] The docs say that SetCurrentDirectory/GetCurrentDirectory
    //           are not thread-safe but we use a lock so ours are.
    void os_set_working_dir(string dir);

    // Get a list of parsed command line arguments excluding the first one - normally the first one
    // is the exe name - but you can get that with os_get_current_module().
    // Note: Don't free the result of this function.
    array<string> os_get_command_line_arguments();

    // The number of threads which can execute concurrently on the current hardware (may be different from the number of cores because of hyperthreads).
    u32 os_get_hardware_concurrency();

    // Returns an ID which uniquely identifies the current process on the system
    u32 os_get_pid();

    // Reads input from the console (at most 1 KiB).
    // Subsequent calls overwrite an internal buffer, so you need to save the information before that.
    // Note: Don't free the result of this function.
    string os_read_from_console();

    struct os_get_env_result {
        string Value;
        bool Success;
    };

    // Get the value of an environment variable, returns true if found.
    // If not found and silent is false, logs warning.
    // The caller is responsible for freeing the returned string.
    // @TODO: Cache this, then we would return a string that needn't be freed and avoid allocations when calling this function multiple times.
    [[nodiscard("Leak")]] os_get_env_result os_get_env(string name, bool silent = false);

    // Sets a variable (creates if it doesn't exist yet) in the current process' environment
    void os_set_env(string name, string value);

    // Remove a variable from the current process' environment
    void os_remove_env(string name);

    // Returns the content stored in the clipboard as a utf8 string.
    // The caller is responsible for freeing.
    [[nodiscard("Leak")]] string os_get_clipboard_content();

    // Sets the clipboard content (expects a utf8 string).
    void os_set_clipboard_content(string content);
}

struct win32_common_state {
    static constexpr s64 CONSOLE_BUFFER_SIZE = 1_KiB;

    char CinBuffer[CONSOLE_BUFFER_SIZE];
    char CoutBuffer[CONSOLE_BUFFER_SIZE];
    char CerrBuffer[CONSOLE_BUFFER_SIZE];

    HANDLE CinHandle, CoutHandle, CerrHandle;
    mutex CoutMutex, CinMutex;

    array<delegate<void()>> ExitFunctions;  // Stores any functions to be called before the program terminates (naturally or by os_exit(exitCode))
    mutex ExitScheduleMutex;                // Used when modifying the ExitFunctions array

    LARGE_INTEGER PerformanceFrequency;  // Used to time stuff

    string ModuleName;  // Caches the module name (retrieve this with os_get_current_module())

    string WorkingDir;  // Caches the working dir (query/modify this with os_get_working_dir(), os_set_working_dir())
    mutex WorkingDirMutex;

    array<string> Argv;
};

// :GlobalStateNoConstructors:
// We create a byte array which is large enough to hold all global variables because
// that avoids the C++ default constructor erasing the state of the struct.
// We initialize it before we call any C++ constructors in the linker table
// (see some stuff we do in exe_main.cpp in lstd/platform/windows_no_crt).
byte State[sizeof(win32_common_state)];

// Short-hand macro for sanity
#define S ((win32_common_state *) &State[0])
#define PERSISTENT platform_get_persistent_allocator()
#define TEMP platform_get_temporary_allocator()

wchar *utf8_to_utf16(string str, allocator alloc = {}) { return platform_utf8_to_utf16(str, alloc); }
string utf16_to_utf8(const wchar *str, allocator alloc = {}) { return platform_utf16_to_utf8(str, alloc); }

void report_warning_no_allocations(string message) {
    DWORD ignored;

    string preMessage = ">>> Warning (in windows_common.cpp): ";
    WriteFile(S->CerrHandle, preMessage.Data, (DWORD) preMessage.Count, &ignored, null);

    WriteFile(S->CerrHandle, message.Data, (DWORD) message.Count, &ignored, null);

    string postMessage = ".\n";
    WriteFile(S->CerrHandle, postMessage.Data, (DWORD) postMessage.Count, &ignored, null);
}

extern "C" bool lstd_init_global();

// This zeroes out the global variables (stored in State) and initializes the mutexes
void init_global_vars() {
    zero_memory(&State, sizeof(win32_common_state));

    platform_init_allocators();

    // Init mutexes
    S->CinMutex          = create_mutex();
    S->CoutMutex         = create_mutex();
    S->ExitScheduleMutex = create_mutex();
    S->WorkingDirMutex   = create_mutex();
#if defined DEBUG_MEMORY
    // @Cleanup
    if (lstd_init_global()) {
        DEBUG_memory = malloc<debug_memory>({.Alloc = PERSISTENT});  // @Leak This is ok
        new (DEBUG_memory) debug_memory;
        DEBUG_memory->Mutex = create_mutex();
    } else {
        DEBUG_memory = null;
    }
#endif
}

void setup_console() {
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();

        // set the screen buffer to be big enough to let us scroll text
        CONSOLE_SCREEN_BUFFER_INFO cInfo;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cInfo);
        cInfo.dwSize.Y = 500;
        SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), cInfo.dwSize);
    }

    S->CinHandle  = GetStdHandle(STD_INPUT_HANDLE);
    S->CoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    S->CerrHandle = GetStdHandle(STD_ERROR_HANDLE);

    if (!SetConsoleOutputCP(CP_UTF8)) {
        report_warning_no_allocations("Couldn't set console code page to UTF8 - some characters might be messed up");
    }

    // Enable ANSI escape sequences for the console
    DWORD dw = 0;
    GetConsoleMode(S->CoutHandle, &dw);
    SetConsoleMode(S->CoutHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    GetConsoleMode(S->CerrHandle, &dw);
    SetConsoleMode(S->CerrHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

constexpr u32 ERROR_INSUFFICIENT_BUFFER = 122;

void get_module_name() {
    // Get the module name
    wchar *buffer = malloc<wchar>({.Count = MAX_PATH, .Alloc = PERSISTENT});
    defer(free(buffer));

    s64 reserved = MAX_PATH;

    while (true) {
        s64 written = GetModuleFileNameW(MODULE_HANDLE, buffer, (DWORD) reserved);
        if (written == reserved) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                reserved *= 2;
                free(buffer);
                buffer = malloc<wchar>({.Count = reserved, .Alloc = PERSISTENT});
                continue;
            }
        }
        break;
    }

    string moduleName = utf16_to_utf8(buffer);
    PUSH_ALLOC(PERSISTENT) {
        S->ModuleName = path_normalize(moduleName);
    }
}

void parse_arguments() {
    // Get the arguments
    wchar **argv;
    s32 argc;

    // @Cleanup @DependencyCleanup: Parse arguments ourselves? We depend on this function which
    // is in a library we reference ONLY because of this one function.
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == null) {
        report_warning_no_allocations("Couldn't parse command line arguments, os_get_command_line_arguments() will return an empty array in all cases");
        return;
    }

    defer(LocalFree(argv));

    PUSH_ALLOC(PERSISTENT) {
        resize(&S->Argv, argc - 1);
    }

    // Loop over all arguments and add them, skip the .exe name
    For(range(1, argc)) add(&S->Argv, utf16_to_utf8(argv[it], PERSISTENT));
}

export {
    // This needs to be called when our program runs, but also when a new thread starts!
    // See windows_common.cpp for implementation details.
    // Note: You shouldn't ever call this.
    void platform_init_context() {
        context newContext  = {};
        newContext.ThreadID = GetCurrentThreadId();
        newContext.Log      = &cout;
        OVERRIDE_CONTEXT(newContext);

        *const_cast<allocator *>(&TemporaryAllocator) = {default_temp_allocator, (void *) &TemporaryAllocatorData};
    }

    //
    // Initializes the state we need to function.
    //
    void platform_init_global_state() {
        init_global_vars();

        setup_console();

        get_module_name();

        parse_arguments();

        QueryPerformanceFrequency(&S->PerformanceFrequency);
    }

    //
    // Reports leaks, uninitializes mutexes.
    //
    void platform_uninit_state() {
#if defined DEBUG_MEMORY
        if (lstd_init_global()) {
        } else {
            // Now we check for memory leaks.
            // Yes, the OS claims back all the memory the program has allocated anyway, and we are not promoting C++ style RAII
            // which make even program termination slow, we are just providing this information to the user because they might
            // want to load/unload DLLs during the runtime of the application, and those DLLs might use all kinds of complex
            // cross-boundary memory stuff things, etc. This is useful for debugging crashes related to that.
            if (DEBUG_memory->CheckForLeaksAtTermination) {
                DEBUG_memory->report_leaks();
            }
        }
#endif

        // Uninit mutexes
        free_mutex(&S->CinMutex);
        free_mutex(&S->CoutMutex);
        free_mutex(&S->ExitScheduleMutex);
        free_mutex(&S->WorkingDirMutex);
#if defined DEBUG_MEMORY
        if (lstd_init_global()) {
            free_mutex(&DEBUG_memory->Mutex);
        }
#endif
    }
}

void exit(s32 exitCode) {
    // :PlatformExitTermination

    // We can't call this from a DLL because of ExitProcess.
    // Search for :PlatformExitTermination to see the other place we call this set of functions.
    exit_call_scheduled_functions();
    win32_monitor_uninit();
    win32_window_uninit();
    platform_uninit_state();

    ExitProcess(exitCode);
}

void abort() {
    ExitProcess(3);
}

void exit_schedule(const delegate<void()> &function) {
    lock(&S->ExitScheduleMutex);

    // @Cleanup Lock-free list
    PUSH_ALLOC(PERSISTENT) {
        add(&S->ExitFunctions, function);
    }

    unlock(&S->ExitScheduleMutex);
}

void exit_call_scheduled_functions() {
    lock(&S->ExitScheduleMutex);

    For(S->ExitFunctions) it();

    unlock(&S->ExitScheduleMutex);
}

array<delegate<void()>> *exit_get_scheduled_functions() {
    return &S->ExitFunctions;
}

time_t os_get_time() {
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return count.QuadPart;
}

f64 os_time_to_seconds(time_t time) {
    return (f64) time / S->PerformanceFrequency.QuadPart;
}

string os_get_current_module() {
    return S->ModuleName;
}

string os_get_working_dir() {
    DWORD required = GetCurrentDirectoryW(0, null);

    auto *dir16 = malloc<wchar>({.Count = required + 1, .Alloc = TEMP});
    if (!GetCurrentDirectoryW(required + 1, dir16)) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "GetCurrentDirectory");
        return "";
    }

    lock(&S->WorkingDirMutex);
    defer(unlock(&S->WorkingDirMutex));

    string workingDir = utf16_to_utf8(dir16);
    PUSH_ALLOC(PERSISTENT) {
        S->WorkingDir = path_normalize(workingDir);
    }
    return S->WorkingDir;
}

void os_set_working_dir(string dir) {
    assert(path_is_absolute(dir));

    WIN32_CHECK_BOOL(r, SetCurrentDirectoryW(utf8_to_utf16(dir)));

    lock(&S->WorkingDirMutex);
    defer(unlock(&S->WorkingDirMutex));

    PUSH_ALLOC(PERSISTENT) {
        S->WorkingDir = clone(dir);
    }
}

constexpr u32 ERROR_ENVVAR_NOT_FOUND = 203;

//
// @TODO: Cache environment variables when running the program in order to avoid allocating.
// Store them null-terminated in the cache, to avoid callers which expect C style strings having to convert.
//
[[nodiscard("Leak")]] os_get_env_result os_get_env(string name, bool silent) {
    auto *name16 = utf8_to_utf16(name, PERSISTENT);
    defer(free(name16));

    DWORD bufferSize = 65535;  // Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx

    auto *buffer = malloc<wchar>({.Count = bufferSize, .Alloc = TEMP});
    auto r       = GetEnvironmentVariableW(name16, buffer, bufferSize);

    if (r == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
        if (!silent) {
            platform_report_error(tsprint("Couldn't find environment variable with value \"{}\"", name));
        }
        return {"", false};
    }

    // 65535 may be the limit but let's not take risks
    if (r > bufferSize) {
        buffer = malloc<wchar>({.Count = r, .Alloc = TEMP});
        GetEnvironmentVariableW(name16, buffer, r);
        bufferSize = r;

        // Possible to fail a second time ?
    }

    return {utf16_to_utf8(buffer, PERSISTENT), true};
}

void os_set_env(string name, string value) {
    if (string_length(value) > 32767) {
        // @Cleanup: The docs say windows doesn't allow that but we should test it.
        assert(false);
    }

    WIN32_CHECK_BOOL(r, SetEnvironmentVariableW(utf8_to_utf16(name), utf8_to_utf16(value)));
}

void os_remove_env(string name) {
    WIN32_CHECK_BOOL(r, SetEnvironmentVariableW(utf8_to_utf16(name), null));
}

[[nodiscard("Leak")]] string os_get_clipboard_content() {
    if (!OpenClipboard(null)) {
        platform_report_error("Failed to open clipboard");
        return "";
    }
    defer(CloseClipboard());

    HANDLE object = GetClipboardData(CF_UNICODETEXT);
    if (!object) {
        platform_report_error("Failed to convert clipboard to string");
        return "";
    }

    auto *clipboard16 = (wchar *) GlobalLock(object);
    if (!clipboard16) {
        platform_report_error("Failed to lock global handle");
        return "";
    }
    defer(GlobalUnlock(object));

    return utf16_to_utf8(clipboard16, PERSISTENT);
}

void os_set_clipboard_content(string content) {
    HANDLE object = GlobalAlloc(GMEM_MOVEABLE, string_length(content) * 2 * sizeof(wchar));
    if (!object) {
        platform_report_error("Failed to open clipboard");
        return;
    }
    defer(GlobalFree(object));

    auto *clipboard16 = (wchar *) GlobalLock(object);
    if (!clipboard16) {
        platform_report_error("Failed to lock global handle");
        return;
    }

    utf8_to_utf16(content.Data, string_length(content), clipboard16);
    GlobalUnlock(object);

    if (!OpenClipboard(null)) {
        platform_report_error("Failed to open clipboard.");
        return;
    }
    defer(CloseClipboard());

    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, object);
    CloseClipboard();
}

array<string> os_get_command_line_arguments() { return S->Argv; }

u32 os_get_hardware_concurrency() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
}

u32 os_get_pid() { return (u32) GetCurrentProcessId(); }

string os_read_from_console() {
    DWORD read;
    ReadFile(S->CinHandle, S->CinBuffer, (DWORD) S->CONSOLE_BUFFER_SIZE, &read, null);
    return string(S->CinBuffer, (s64) read);
}

// @TODO @Clarity: Print more useful message about the path
// :CopyAndPaste from path.nt
#define CREATE_MAPPING_CHECKED(handleName, call)                                                               \
    HANDLE handleName = call;                                                                                  \
    if (!handleName) {                                                                                         \
        string extendedCallSite = sprint("{}\n        (the name was: {!YELLOW}\"{}\"{!GRAY})\n", #call, name); \
        char *cStr              = string_to_c_string(extendedCallSite);                                        \
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), cStr);                                \
        free(cStr);                                                                                            \
        free(extendedCallSite);                                                                                \
        return;                                                                                                \
    }

[[nodiscard("Leak")]] os_read_file_result os_read_entire_file(string path) {
    os_read_file_result fail = {string(), false};
    CREATE_FILE_HANDLE_CHECKED(file, CreateFileW(utf8_to_utf16(path), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null), fail);
    defer(CloseHandle(file));

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);

    string result;
    resize(&result, size.QuadPart);
    DWORD bytesRead;
    if (!ReadFile(file, result.Data, (u32) size.QuadPart, &bytesRead, null)) return {{}, false};
    assert(size.QuadPart == bytesRead);

    result.Count = bytesRead;
    return {result, true};
}

bool os_write_to_file(string path, string contents, file_write_mode mode) {
    CREATE_FILE_HANDLE_CHECKED(file, CreateFileW(utf8_to_utf16(path), GENERIC_WRITE, 0, null, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, null), false);
    defer(CloseHandle(file));

    LARGE_INTEGER pointer = {};
    pointer.QuadPart      = 0;
    if (mode == file_write_mode::Append) SetFilePointerEx(file, pointer, null, FILE_END);
    if (mode == file_write_mode::Overwrite_Entire) SetEndOfFile(file);

    DWORD bytesWritten;
    if (!WriteFile(file, contents.Data, (u32) contents.Count, &bytesWritten, null)) return false;
    if (bytesWritten != contents.Count) return false;
    return true;
}

void console::write(const char *data, s64 size) {
    if (LockMutex) lock(&S->CoutMutex);

    if (size > Available) {
        flush();
    }

    copy_memory(Current, data, size);

    Current += size;
    Available -= size;

    if (LockMutex) unlock(&S->CoutMutex);
}

void console::flush() {
    if (LockMutex) lock(&S->CoutMutex);

    if (!Buffer) {
        if (OutputType == console::COUT) {
            Buffer = Current = S->CoutBuffer;
        } else {
            Buffer = Current = S->CerrBuffer;
        }

        BufferSize = Available = S->CONSOLE_BUFFER_SIZE;
    }

    HANDLE target = OutputType == console::COUT ? S->CoutHandle : S->CerrHandle;

    DWORD ignored;
    WriteFile(target, Buffer, (DWORD) (BufferSize - Available), &ignored, null);

    Current   = Buffer;
    Available = S->CONSOLE_BUFFER_SIZE;

    if (LockMutex) unlock(&S->CoutMutex);
}

LSTD_END_NAMESPACE
