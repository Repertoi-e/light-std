module;

#include "lstd/io.h"
#include "lstd/memory/array.h"
#include "lstd/memory/delegate.h"
#include "lstd/memory/string.h"
#include "lstd/common/windows.h"  // Declarations of Win32 functions

//
// Simple wrapper around dynamic libraries and getting addresses of procedures.
//

export module os.win64.common;

import os.win64.memory;

import fmt;
import path;

extern "C" IMAGE_DOS_HEADER __ImageBase;
#define MODULE_HANDLE ((HMODULE) &__ImageBase)

LSTD_BEGIN_NAMESPACE

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
    void os_set_working_dir(const string &dir);

    // Get a list of parsed command line arguments excluding the first one - normally the first one
    // is the exe name - but you can get that with os_get_current_module().
    // Note: Don't free the result of this function.
    array<string> os_get_command_line_arguments();

    // Returns an ID which uniquely identifies the current process on the system
    u32 os_get_pid();

    // Reads input from the console (at most 1 KiB).
    // Subsequent calls overwrite an internal buffer, so you need to save the information before that.
    // Note: Don't free the result of this function.
    bytes os_read_from_console();

    struct os_get_env_result {
        string Value;
        bool Success;
    };

    // Get the value of an environment variable, returns true if found.
    // If not found and silent is false, logs warning.
    // The caller is responsible for freeing the returned string.
    // @TODO: Cache this, then we would return a string that needn't be freed and avoid allocations when calling this function multiple times.
    [[nodiscard("Leak")]] os_get_env_result os_get_env(const string &name, bool silent = false);

    // Sets a variable (creates if it doesn't exist yet) in the current process' environment
    void os_set_env(const string &name, const string &value);

    // Remove a variable from the current process' environment
    void os_remove_env(const string &name);

    // Returns the content stored in the clipboard as a utf8 string.
    // The caller is responsible for freeing.
    [[nodiscard("Leak")]] string os_get_clipboard_content();

    // Sets the clipboard content (expects a utf8 string).
    void os_set_clipboard_content(const string &content);
}

struct win64_common_state {
    static constexpr s64 CONSOLE_BUFFER_SIZE = 1_KiB;

    byte CinBuffer[CONSOLE_BUFFER_SIZE];
    byte CoutBuffer[CONSOLE_BUFFER_SIZE];
    byte CerrBuffer[CONSOLE_BUFFER_SIZE];

    HANDLE CinHandle, CoutHandle, CerrHandle;
    thread::mutex CoutMutex, CinMutex;

    array<delegate<void()>> ExitFunctions;  // Stores any functions to be called before the program terminates (naturally or by os_exit(exitCode))
    thread::mutex ExitScheduleMutex;        // Used when modifying the ExitFunctions array

    LARGE_INTEGER PerformanceFrequency;  // Used to time stuff

    string ModuleName;  // Caches the module name (retrieve this with os_get_current_module())

    string WorkingDir;  // Caches the working dir (query/modify this with os_get_working_dir(), os_set_working_dir())
    thread::mutex WorkingDirMutex;

    array<string> Argv;
};

// :GlobalStateNoConstructors:
// We create a byte array which large enough to hold all global variables because
// that avoids C++ constructors erasing the state we initialize before any C++ constructors are called.
// Some further explanation... We need to initialize this before main is run. We need to initialize this
// before even constructors for global variables (refered to as C++ constructors) are called (which may
// rely on e.g. the Context being initialized). This is analogous to the stuff CRT runs before main is called.
// Except that we don't link against the CRT (that's why we even have to "call" the constructors ourselves,
// using linker magic - take a look at the exe_main.cpp in no_crt).
byte State[sizeof(win64_common_state)];

// Short-hand macro for sanity
#define S ((win64_common_state *) &State[0])
#define PERSISTENT internal::platform_get_persistent_allocator()
#define TEMP internal::platform_get_temporary_allocator()

utf16 *utf8_to_utf16(const string &str, allocator alloc = {}) { return internal::platform_utf8_to_utf16(str, alloc); }
string utf16_to_utf8(const utf16 *str, allocator alloc = {}) { return internal::platform_utf16_to_utf8(str, alloc); }

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
    zero_memory(&State, sizeof(win64_common_state));

    internal::platform_init_allocators();

    // Init mutexes
    S->CinMutex.init();
    S->CoutMutex.init();
    S->ExitScheduleMutex.init();
    S->WorkingDirMutex.init();
#if defined DEBUG_MEMORY
    // @Cleanup
    if (lstd_init_global()) {
        DEBUG_memory = allocate<debug_memory>({.Alloc = PERSISTENT});  // @Leak This is ok
        new (DEBUG_memory) debug_memory;
        DEBUG_memory->Mutex.init();
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

    S->CinHandle = GetStdHandle(STD_INPUT_HANDLE);
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
    utf16 *buffer = allocate_array<utf16>(MAX_PATH, {.Alloc = PERSISTENT});
    defer(free(buffer));

    s64 reserved = MAX_PATH;

    while (true) {
        s64 written = GetModuleFileNameW(MODULE_HANDLE, buffer, (DWORD) reserved);
        if (written == reserved) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                reserved *= 2;
                free(buffer);
                buffer = allocate_array<utf16>(reserved, {.Alloc = PERSISTENT});
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
    utf16 **argv;
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
        array_reserve(S->Argv, argc - 1);
    }

    // Loop over all arguments and add them, skip the .exe name
    For(range(1, argc)) array_append(S->Argv, utf16_to_utf8(argv[it], PERSISTENT));
}

export namespace internal {

// This needs to be called when our program runs, but also when a new thread starts!
// See windows_common.cpp for implementation details.
// Note: You shouldn't ever call this.
void platform_init_context() {
    context newContext = {};
    newContext.ThreadID = thread::id((u64) GetCurrentThreadId());
    newContext.TempAlloc = {default_temp_allocator, (void *) &__TempAllocData};
    newContext.Log = &cout;
    OVERRIDE_CONTEXT(newContext);
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
    S->CinMutex.release();
    S->CoutMutex.release();
    S->ExitScheduleMutex.release();
    S->WorkingDirMutex.release();
#if defined DEBUG_MEMORY
    if (lstd_init_global()) {
        DEBUG_memory->Mutex.release();
    }
#endif
}
}  // namespace internal

export {
    void exit(s32 exitCode) {
        // :PlatformExitTermination
        exit_call_scheduled_functions();
        internal::platform_uninit_state();
        ExitProcess(exitCode);
    }

    void abort() {
        ExitProcess(3);
    }

    void exit_schedule(const delegate<void()> &function) {
        thread::scoped_lock _(&S->ExitScheduleMutex);

        PUSH_ALLOC(PERSISTENT) {
            array_append(S->ExitFunctions, function);
        }
    }

    void exit_call_scheduled_functions() {
        thread::scoped_lock _(&S->ExitScheduleMutex);
        For(S->ExitFunctions) it();
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

        auto *dir16 = allocate_array<utf16>(required + 1, {.Alloc = TEMP});
        if (!GetCurrentDirectoryW(required + 1, dir16)) {
            windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "GetCurrentDirectory");
            return "";
        }

        thread::scoped_lock _(&S->WorkingDirMutex);

        string workingDir = utf16_to_utf8(dir16);
        PUSH_ALLOC(PERSISTENT) {
            S->WorkingDir = path_normalize(workingDir);
        }
        return S->WorkingDir;
    }

    void os_set_working_dir(const string &dir) {
        assert(path_is_absolute(dir));

        WIN_CHECKBOOL(SetCurrentDirectoryW(utf8_to_utf16(dir)));

        thread::scoped_lock _(&S->WorkingDirMutex);
        PUSH_ALLOC(PERSISTENT) {
            clone(&S->WorkingDir, dir);
        }
    }

    constexpr u32 ERROR_ENVVAR_NOT_FOUND = 203;

    //
    // @TODO: Cache environment variables when running the program in order to avoid allocating.
    //
    [[nodiscard("Leak")]] os_get_env_result os_get_env(const string &name, bool silent) {
        auto *name16 = utf8_to_utf16(name, PERSISTENT);
        defer(free(name16));

        DWORD bufferSize = 65535;  // Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx

        auto *buffer = allocate_array<utf16>(bufferSize, {.Alloc = TEMP});
        auto r = GetEnvironmentVariableW(name16, buffer, bufferSize);

        if (r == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
            if (!silent) {
                internal::platform_report_error(tsprint("Couldn't find environment variable with value \"{}\"", name));
            }
            return {"", false};
        }

        // 65535 may be the limit but let's not take risks
        if (r > bufferSize) {
            buffer = allocate_array<utf16>(r, {.Alloc = TEMP});
            GetEnvironmentVariableW(name16, buffer, r);
            bufferSize = r;

            // Possible to fail a second time ?
        }

        return {utf16_to_utf8(buffer, PERSISTENT), true};
    }

    void os_set_env(const string &name, const string &value) {
        if (value.Length > 32767) {
            // @Cleanup: The docs say windows doesn't allow that but we should test it.
            assert(false);
        }

        WIN_CHECKBOOL(SetEnvironmentVariableW(utf8_to_utf16(name), utf8_to_utf16(value)));
    }

    void os_remove_env(const string &name) {
        WIN_CHECKBOOL(SetEnvironmentVariableW(utf8_to_utf16(name), null));
    }

    [[nodiscard("Leak")]] string os_get_clipboard_content() {
        if (!OpenClipboard(null)) {
            internal::platform_report_error("Failed to open clipboard");
            return "";
        }
        defer(CloseClipboard());

        HANDLE object = GetClipboardData(CF_UNICODETEXT);
        if (!object) {
            internal::platform_report_error("Failed to convert clipboard to string");
            return "";
        }

        auto *clipboard16 = (utf16 *) GlobalLock(object);
        if (!clipboard16) {
            internal::platform_report_error("Failed to lock global handle");
            return "";
        }
        defer(GlobalUnlock(object));

        return utf16_to_utf8(clipboard16, PERSISTENT);
    }

    void os_set_clipboard_content(const string &content) {
        HANDLE object = GlobalAlloc(GMEM_MOVEABLE, content.Length * 2 * sizeof(utf16));
        if (!object) {
            internal::platform_report_error("Failed to open clipboard");
            return;
        }
        defer(GlobalFree(object));

        auto *clipboard16 = (utf16 *) GlobalLock(object);
        if (!clipboard16) {
            internal::platform_report_error("Failed to lock global handle");
            return;
        }

        utf8_to_utf16(content.Data, content.Length, clipboard16);
        GlobalUnlock(object);

        if (!OpenClipboard(null)) {
            internal::platform_report_error("Failed to open clipboard.");
            return;
        }
        defer(CloseClipboard());

        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, object);
        CloseClipboard();
    }

    array<string> os_get_command_line_arguments() { return S->Argv; }

    u32 os_get_pid() { return (u32) GetCurrentProcessId(); }

    bytes os_read_from_console() {
        DWORD read;
        ReadFile(S->CinHandle, S->CinBuffer, (DWORD) S->CONSOLE_BUFFER_SIZE, &read, null);
        return bytes(S->CinBuffer, (s64) read);
    }

    void console_writer::write(const byte *data, s64 size) {
        thread::mutex *mutex = null;
        if (LockMutex) mutex = &S->CoutMutex;
        thread::scoped_lock _(mutex);

        if (size > Available) {
            flush();
        }

        copy_memory(Current, data, size);

        Current += size;
        Available -= size;
    }

    void console_writer::flush() {
        thread::mutex *mutex = null;
        if (LockMutex) mutex = &S->CoutMutex;
        thread::scoped_lock _(mutex);

        if (!Buffer) {
            if (OutputType == console_writer::COUT) {
                Buffer = Current = S->CoutBuffer;
            } else {
                Buffer = Current = S->CerrBuffer;
            }

            BufferSize = Available = S->CONSOLE_BUFFER_SIZE;
        }

        HANDLE target = OutputType == console_writer::COUT ? S->CoutHandle : S->CerrHandle;

        DWORD ignored;
        WriteFile(target, Buffer, (DWORD)(BufferSize - Available), &ignored, null);

        Current = Buffer;
        Available = S->CONSOLE_BUFFER_SIZE;
    }
}

LSTD_END_NAMESPACE
