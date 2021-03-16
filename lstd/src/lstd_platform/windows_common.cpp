#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/io.h"
#include "lstd/memory/dynamic_library.h"
#include "lstd/memory/guid.h"
#include "lstd/os.h"
#include "lstd/types/windows.h"  // Declarations of API functions
#include "lstd/types/windows_status_codes.h"

import path;
import fmt;

//
// This is here to assist cases where you want to share the memory between two modules (e.g. an exe and a dll or multiple dlls, etc.)
// By default, when you link lstd with the dll, each dll gets its own global state (global allocator, debug memory info, etc.),
// which means that allocations done in different modules are incompatible. If you provide a symbol lstd_dont_initialize_global_state_stub
// with the value "true" we don't initialize that global state (instead we leave it as null). That means that YOU MUST initialize it yourself!
// You must initialize the following global variables by passing the values from the "host" to the "guest" module:
//  - DEBUG_memory     (a global pointer, by default we allocate it)
//
// @Volatile: As we add more global state.
//
// Why do we do this?
// In another project (light-std-graphics) I have an exe which serves as the engine, and loads dlls (the game). We do this to support hot-loading
// so we can change the game code without closing the window. The game (dll) allocates memory and needs to do that from the engine's allocator and debug memory,
// otherwise problems occur when hot-loading a new dll.
//
// @Cleanup @Cleanup @Cleanup @Cleanup @Cleanup @Cleanup @Cleanup @Cleanup  There should be a better way and we should get rid of this.
//                                                                          I haven't thought much about it yet.
extern "C" bool lstd_init_global();

// If the user didn't provide a definition for lstd_init_global, the linker shouldn't complain,
// but instead provide a stub function which returns true.
extern "C" bool lstd_init_global_stub() { return true; }
#pragma comment(linker, "/ALTERNATENAME:lstd_init_global=lstd_init_global_stub")

LSTD_BEGIN_NAMESPACE

extern "C" IMAGE_DOS_HEADER __ImageBase;
#define MODULE_HANDLE ((HMODULE) &__ImageBase)

struct win64_common_state {
    allocator PersistentAlloc;  // Used to store global state, a tlsf allocator
    thread::mutex PersistentAllocMutex;

    allocator TempAlloc;  // Used for temporary storage (e.g. converting strings from utf8 to utf16 for windows calls).
                          // Memory returned is valid until the next TempAlloc call.
    thread::mutex TempAllocMutex;

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

// We create a byte array which large enough to hold all global variables because
// that avoids C++ constructors erasing the state we initialize before any C++ constructors are called.
// Some further explanation... We need to initialize this before main is run. We need to initialize this
// before even constructors for global variables (refered to as C++ constructors) are called (which may
// rely on e.g. the Context being initialized). This is analogous to the stuff CRT runs before main is called.
// Except that we don't link against the CRT (that's why we even have to "call" the constructors ourselves,
// using linker magic - take a look at the exe_main.cpp in no_crt).
file_scope byte State[sizeof(win64_common_state)];

// Short-hand macro for sanity
#define S ((win64_common_state *) &State[0])

// These functions are used by other windows platform files.
allocator win64_get_persistent_allocator() { return S->PersistentAlloc; }
allocator win64_get_temporary_allocator() { return S->TempAlloc; }

file_scope void report_warning_no_allocations(string message) {
    DWORD ignored;

    string preMessage = ">>> Warning (in windows_common.cpp): ";
    WriteFile(S->CerrHandle, preMessage.Data, (DWORD) preMessage.Count, &ignored, null);

    WriteFile(S->CerrHandle, message.Data, (DWORD) message.Count, &ignored, null);

    string postMessage = ".\n";
    WriteFile(S->CerrHandle, postMessage.Data, (DWORD) postMessage.Count, &ignored, null);
}

// @TODO: Add option to print call stack?
void platform_report_warning(string message, source_location loc = source_location::current()) {
    print(">>> {!YELLOW}Platform warning{!} {}:{} (in function: {}): {}.\n", loc.File, loc.Line, loc.Function, message);
}

// @TODO: Add option to print call stack?
void platform_report_error(string message, source_location loc = source_location::current()) {
    print(">>> {!RED}Platform error{!} {}:{} (in function: {}): {}.\n", loc.File, loc.Line, loc.Function, message);
}

// @TODO: Make these macros so the user (programmer) can modify them easily.
constexpr s64 TEMPORARY_STORAGE_STARTING_POOL_SIZE = 16_KiB;
constexpr s64 PERSISTENT_STORAGE_STARTING_POOL_SIZE = 1_MiB;

// Some global state about our temporary allocator.
// We don't use the one bundled with the Context because we don't want to mess with the user's memory.
file_scope void *TempStorageBlock;
file_scope s64 TempStorageSize;

void create_temp_storage_block(s64);
void create_persistent_alloc_block(s64);

// An extension to the arena allocator. Calls free_all when not enough space. Because we are not running a game
// there is no clear point at which to free_all the temporary allocator, that's why we assume that no allocation
// made with TempAlloc should persist beyond the next allocation.
void *win64_temp_alloc(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
    auto *data = (arena_allocator_data *) context;

    thread::scoped_lock _(&S->TempAllocMutex);

    auto *result = arena_allocator(mode, context, size, oldMemory, oldSize, options);
    if (mode == allocator_mode::ALLOCATE) {
        if (size > TempStorageSize) {
            // If we try to allocate a block with size bigger than the temporary storage block, we make a new, larger temporary storage block

            platform_report_warning("Not enough memory in the temporary allocator; expanding the pool");

            allocator_remove_pool(S->TempAlloc, TempStorageBlock);
            os_free_block((byte *) TempStorageBlock - sizeof(arena_allocator_data));

            create_temp_storage_block(size * 2);
            result = arena_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
        } else if (!result) {
            // If we couldn't allocate but the temporary storage block has enough space, we just call free_all
            free_all(S->TempAlloc);
            result = arena_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
        }
    }

    return result;
}

file_scope void create_temp_storage_block(s64 size) {
    // We allocate the arena allocator data and the starting pool in one big block in order to reduce fragmentation.
    auto pools = to_stack_array(size);
    auto [data, pool] = os_allocate_packed<arena_allocator_data>(pools);
    S->TempAlloc = {win64_temp_alloc, data};
    allocator_add_pool(S->TempAlloc, pool, size);

    TempStorageBlock = pool;
    TempStorageSize = size; 
}

void *win64_persistent_alloc(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
    auto *data = (tlsf_allocator_data *) context;

    thread::scoped_lock _(&S->PersistentAllocMutex);

    auto *result = tlsf_allocator(mode, context, size, oldMemory, oldSize, options);
    if (mode == allocator_mode::ALLOCATE) {
        if (!result) {
            platform_report_warning("Not enough memory in the persistent allocator; adding a pool");

            create_persistent_alloc_block(size * 3);
        }
        result = tlsf_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
    }
    return result;
}

file_scope void create_persistent_alloc_block(s64 size) {
    // We allocate the arena allocator data and the starting pool in one big block in order to reduce fragmentation.
    auto pools = to_stack_array(size);
    auto [data, pool] = os_allocate_packed<tlsf_allocator_data>(pools);
    S->PersistentAlloc = {win64_persistent_alloc, data};
    allocator_add_pool(S->PersistentAlloc, pool, size);
}

file_scope void init_allocators() {
    create_temp_storage_block(TEMPORARY_STORAGE_STARTING_POOL_SIZE);
    create_persistent_alloc_block(PERSISTENT_STORAGE_STARTING_POOL_SIZE);
}

// This zeroes out the global variables (stored in State) and initializes the mutexes
file_scope void init_global_vars() {
    zero_memory(&State, sizeof(win64_common_state));

    // Init mutexes
    S->PersistentAllocMutex.init();
    S->TempAllocMutex.init();
    S->CinMutex.init();
    S->CoutMutex.init();
    S->ExitScheduleMutex.init();
    S->WorkingDirMutex.init();
#if defined DEBUG_MEMORY
    if (lstd_init_global()) {
        DEBUG_memory = (debug_memory *) os_allocate_block(sizeof(debug_memory));  // @Leak This is ok
        new (DEBUG_memory) debug_memory;
        DEBUG_memory->Mutex.init();
    } else {
        DEBUG_memory = null;
    }
#endif

    init_allocators();
}

// When our program runs, but also needs to happen when a new thread starts!
void win64_common_init_context() {
    context newContext = {};
    newContext.ThreadID = thread::id((u64) GetCurrentThreadId());
    newContext.TempAlloc = {default_temp_allocator, (void *) &__TempAllocData};
    newContext.Log = &cout;
    OVERRIDE_CONTEXT(newContext);
}

void exit_schedule(const delegate<void()> &function) {
    thread::scoped_lock _(&S->ExitScheduleMutex);

    PUSH_ALLOC(S->PersistentAlloc) {
        append(S->ExitFunctions, function);
    }
}

// We supply this as API to the user if they are doing something very hacky..
void exit_call_scheduled_functions() {
    thread::scoped_lock _(&S->ExitScheduleMutex);

    For(S->ExitFunctions) it();
}

// We supply this as API to the user if they are doing something very hacky..
array<delegate<void()>> *exit_get_scheduled_functions() {
    return &S->ExitFunctions;
}

file_scope void uninit_state() {
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

void win64_common_init_context();
void win64_common_init_global_state();

void win64_window_init();
void win64_window_uninit();

// We use to do this on MSVC but since then we no longer link the CRT and do all of this ourselves. (take a look at no_crt/exe_main.cpp)
//
// This trick makes all of the above requirements work on the MSVC compiler.
//
// How it works is described in this awesome article:
// https://www.codeguru.com/cpp/misc/misc/applicationcontrol/article.php/c6945/Running-Code-Before-and-After-Main.htm#page-2
#if COMPILER == MSVC
file_scope s32 c_init() {
    win64_common_init_context();

    win64_common_init_global_state();

    return 0;
}

file_scope s32 tls_init() {
    win64_common_init_context();
    return 0;
}

file_scope s32 pre_termination() {
    exit_call_scheduled_functions();

    uninit_state();
    return 0;
}

typedef s32 cb(void);
#pragma const_seg(".CRT$XIU")
__declspec(allocate(".CRT$XIU")) cb *g_CInit = c_init;
#pragma const_seg()

#pragma const_seg(".CRT$XDU")
__declspec(allocate(".CRT$XDU")) cb *g_TLSInit = tls_init;
#pragma const_seg()

#pragma const_seg(".CRT$XPU")
__declspec(allocate(".CRT$XPU")) cb *g_PreTermination = pre_termination;
#pragma const_seg()

#else
#error @TODO: See how this works on other compilers!
#endif

// Windows uses utf16.. Sigh...
extern "C" {
utf16 *utf8_to_utf16(const string &str, allocator alloc = {}) {
    if (!str.Length) return null;

    if (!alloc) alloc = S->TempAlloc;

    utf16 *result;
    PUSH_ALLOC(alloc) {
        // src.Length * 2 because one unicode character might take 2 wide chars.
        // This is just an approximation, not all space will be used!
        result = allocate_array<utf16>(str.Length * 2 + 1);
    }

    utf8_to_utf16(str.Data, str.Length, result);
    return result;
}
}

string utf16_to_utf8(const utf16 *str, allocator alloc = {}) {
    string result;

    if (!alloc) alloc = S->TempAlloc;

    PUSH_ALLOC(alloc) {
        // String length * 4 because one unicode character might take 4 bytes in utf8.
        // This is just an approximation, not all space will be used!
        reserve(result, c_string_length(str) * 4);
    }

    utf16_to_utf8(str, (utf8 *) result.Data, &result.Count);
    result.Length = utf8_length(result.Data, result.Count);

    return result;
}

file_scope void setup_console() {
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

file_scope void get_module_name() {
    // Get the module name
    utf16 *buffer = allocate_array<utf16>(MAX_PATH, {.Alloc = S->PersistentAlloc});
    defer(free(buffer));

    s64 reserved = MAX_PATH;

    while (true) {
        s64 written = GetModuleFileNameW(MODULE_HANDLE, buffer, (DWORD) reserved);
        if (written == reserved) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                reserved *= 2;
                free(buffer);
                buffer = allocate_array<utf16>(reserved, {.Alloc = S->PersistentAlloc});
                continue;
            }
        }
        break;
    }

    string moduleName = utf16_to_utf8(buffer);
    PUSH_ALLOC(S->PersistentAlloc) {
        S->ModuleName = path_normalize(moduleName);
    }
}

file_scope void parse_arguments() {
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

    PUSH_ALLOC(S->PersistentAlloc) {
        reserve(S->Argv, argc - 1);
    }

    // Loop over all arguments and add them, skip the .exe name
    For(range(1, argc)) append(S->Argv, utf16_to_utf8(argv[it], S->PersistentAlloc));
}

//
// Initializes the state we need to function.
//
void win64_common_init_global_state() {
    init_global_vars();

    setup_console();

    get_module_name();

    parse_arguments();

    QueryPerformanceFrequency(&S->PerformanceFrequency);
}

//
// Implementation of os.h and cout (console_writer.h).
//

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

void *os_allocate_block(s64 size) {
    assert(size < MAX_ALLOCATION_REQUEST);
    return HeapAlloc(GetProcessHeap(), 0, size);
}

// Tests whether the allocation contraction is possible
file_scope bool is_contraction_possible(s64 oldSize) {
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

file_scope void *try_heap_realloc(void *ptr, s64 newSize, bool *reportError) {
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
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "HeapReAlloc");
    }
    return null;
}

s64 os_get_block_size(void *ptr) {
    s64 result = HeapSize(GetProcessHeap(), 0, ptr);
    if (result == -1) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "HeapSize");
        return 0;
    }
    return result;
}

#define CREATE_MAPPING_CHECKED(handleName, call, returnOnFail)                                                 \
    HANDLE handleName = call;                                                                                  \
    if (!handleName) {                                                                                         \
        string extendedCallSite = sprint("{}\n        (the name was: {!YELLOW}\"{}\"{!GRAY})\n", #call, name); \
        defer(free(extendedCallSite));                                                                         \
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), extendedCallSite);                    \
        return returnOnFail;                                                                                   \
    }

void os_write_shared_block(const string &name, void *data, s64 size) {
    CREATE_MAPPING_CHECKED(h, CreateFileMappingW(INVALID_HANDLE_VALUE, null, PAGE_READWRITE, 0, (DWORD) size, utf8_to_utf16(name)), );
    defer(CloseHandle(h));

    void *result = MapViewOfFile(h, FILE_MAP_WRITE, 0, 0, size);
    if (!result) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "MapViewOfFile");
        return;
    }
    copy_memory(result, data, size);
    UnmapViewOfFile(result);
}

void os_read_shared_block(const string &name, void *out, s64 size) {
    CREATE_MAPPING_CHECKED(h, OpenFileMappingW(FILE_MAP_READ, false, utf8_to_utf16(name)), );
    defer(CloseHandle(h));

    void *result = MapViewOfFile(h, FILE_MAP_READ, 0, 0, size);
    if (!result) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "MapViewOfFile");
        return;
    }

    copy_memory(out, result, size);
    UnmapViewOfFile(result);
}

void os_free_block(void *ptr) {
    WIN32_CHECKBOOL(HeapFree(GetProcessHeap(), 0, ptr));
}

void os_exit(s32 exitCode) {
    exit_call_scheduled_functions();
    uninit_state();
    ExitProcess(exitCode);
}

void os_abort() {
    ExitProcess(3);
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

    auto *dir16 = allocate_array<utf16>(required + 1, {.Alloc = S->TempAlloc});
    if (!GetCurrentDirectoryW(required + 1, dir16)) {
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "GetCurrentDirectory");
        return "";
    }

    thread::scoped_lock _(&S->WorkingDirMutex);

    string workingDir = utf16_to_utf8(dir16);
    PUSH_ALLOC(S->PersistentAlloc) {
        S->WorkingDir = path_normalize(workingDir);
    }
    return S->WorkingDir;
}

void os_set_working_dir(const string &dir) {
    assert(path_is_absolute(dir));

    WIN32_CHECKBOOL(SetCurrentDirectoryW(utf8_to_utf16(dir)));

    thread::scoped_lock _(&S->WorkingDirMutex);
    PUSH_ALLOC(S->PersistentAlloc) {
        clone(&S->WorkingDir, dir);
    }
}

//
// @TODO: Cache environment variables when running the program in order to avoid allocating.
//
[[nodiscard("Leak")]] os_get_env_result os_get_env(const string &name, bool silent) {
    auto *name16 = utf8_to_utf16(name, S->PersistentAlloc);
    defer(free(name16));

    DWORD bufferSize = 65535;  // Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx

    auto *buffer = allocate_array<utf16>(bufferSize, {.Alloc = S->TempAlloc});
    auto r = GetEnvironmentVariableW(name16, buffer, bufferSize);

    if (r == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
        if (!silent) {
            platform_report_error(tsprint("Couldn't find environment variable with value \"{}\"", name));
        }
        return {"", false};
    }

    // 65535 may be the limit but let's not take risks
    if (r > bufferSize) {
        buffer = allocate_array<utf16>(r, {.Alloc = S->TempAlloc});
        GetEnvironmentVariableW(name16, buffer, r);
        bufferSize = r;

        // Possible to fail a second time ?
    }

    return {utf16_to_utf8(buffer, S->PersistentAlloc), true};
}

void os_set_env(const string &name, const string &value) {
    if (value.Length > 32767) {
        // @Cleanup: The docs say windows doesn't allow that but we should test it.
        assert(false);
    }

    WIN32_CHECKBOOL(SetEnvironmentVariableW(utf8_to_utf16(name), utf8_to_utf16(value)));
}

void os_remove_env(const string &name) {
    WIN32_CHECKBOOL(SetEnvironmentVariableW(utf8_to_utf16(name), null));
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

    auto *clipboard16 = (utf16 *) GlobalLock(object);
    if (!clipboard16) {
        platform_report_error("Failed to lock global handle");
        return "";
    }
    defer(GlobalUnlock(object));

    return utf16_to_utf8(clipboard16, S->PersistentAlloc);
}

void os_set_clipboard_content(const string &content) {
    HANDLE object = GlobalAlloc(GMEM_MOVEABLE, content.Length * 2 * sizeof(utf16));
    if (!object) {
        platform_report_error("Failed to open clipboard");
        return;
    }
    defer(GlobalFree(object));

    auto *clipboard16 = (utf16 *) GlobalLock(object);
    if (!clipboard16) {
        platform_report_error("Failed to lock global handle");
        return;
    }

    utf8_to_utf16(content.Data, content.Length, clipboard16);
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

u32 os_get_pid() { return (u32) GetCurrentProcessId(); }

//
// Implementation of guid.h
//

guid guid_new() {
    GUID g;
    CoCreateGuid(&g);

    auto data = to_stack_array((byte)((g.Data1 >> 24) & 0xFF), (byte)((g.Data1 >> 16) & 0xFF),
                               (byte)((g.Data1 >> 8) & 0xFF), (byte)((g.Data1) & 0xff),

                               (byte)((g.Data2 >> 8) & 0xFF), (byte)((g.Data2) & 0xff),

                               (byte)((g.Data3 >> 8) & 0xFF), (byte)((g.Data3) & 0xFF),

                               (byte) g.Data4[0], (byte) g.Data4[1], (byte) g.Data4[2], (byte) g.Data4[3],
                               (byte) g.Data4[4], (byte) g.Data4[5], (byte) g.Data4[6], (byte) g.Data4[7]);
    return guid(data);
}

//
// Implementation of dynamic_library.h
//
bool dynamic_library::load(const string &name) {
    Handle = (void *) LoadLibraryW(utf8_to_utf16(name));
    return Handle;
}

void *dynamic_library::get_symbol(const string &name) {
    return (void *) GetProcAddress((HMODULE) Handle, (LPCSTR) to_c_string(name, S->TempAlloc));
}

void dynamic_library::close() {
    if (Handle) {
        FreeLibrary((HMODULE) Handle);
        Handle = null;
    }
}

LSTD_END_NAMESPACE

#endif