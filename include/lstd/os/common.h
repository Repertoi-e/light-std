#pragma once

//
// Simple common functions that require OS-specific functionality.
//

#include "../fmt.h"
#include "../variant.h"
#include "../memory.h"
#include "../writer.h"
#include "thread.h"
#include "path.h"

#include "../common/numeric/types.h"

#if not defined LSTD_NO_CRT
#include <stdlib.h>
#endif

LSTD_BEGIN_NAMESPACE

enum class file_write_mode {
  Append = 0,

  // If the file is 50 bytes and you write 20,
  // "Overwrite" keeps those 30 bytes at the end
  // while "Overwrite_Entire" deletes them.
  Overwrite,
  Overwrite_Entire,
};

// Reads entire file into memory (no async variant available at the moment).
mark_as_leak optional<string> os_read_entire_file(string path);

// Write _contents_ to a file.
// _mode_ determines if the content should be appended or overwritten. See
// _file_write_mode_ above.
//
// Returns true on success.
bool os_write_to_file(string path, string contents, file_write_mode mode);

// Returns a time stamp that can be used for time-interval measurements
time_t os_get_time();

// Converts a time stamp acquired by os_get_time() to seconds
f64 os_time_to_seconds(time_t time);

// Returns the path of the current module (executable or dynamic library).
// Full dir + name.
//
// Note: Don't free the result of this function, not a leak.
string os_get_current_module();

// Returns the current directory of the current process.
// [Thread Safe] The docs say that for e.g. SetCurrentDirectory/GetCurrentDirectory on Windows,
//               are not thread-safe but we use a lock so ours are. Probably applies to other platforms as well.
// Note: Don't free the result of this function, not a leak.
string os_get_working_dir();

// Sets the current directory of the current process.
// [Thread Safe] The docs say that for e.g. SetCurrentDirectory/GetCurrentDirectory on Windows,
//               are not thread-safe but we use a lock so ours are. Probably applies to other platforms as well.
void os_set_working_dir(string dir);

// Get a list of parsed command line arguments excluding the first one -
// normally the first one is the exe name - but you can get that with
// os_get_current_module(). Note: Don't free the result of this function, not a leak.
array<string> os_get_command_line_arguments();

// The number of threads which can execute concurrently on the current
// hardware (may be different from the number of cores because of
// hyperthreads).
u32 os_get_hardware_concurrency();

// Returns an ID which uniquely identifies the current process on the system
u32 os_get_pid();

// Returns an ID which uniquely identifies the current thread 
u64 os_get_current_thread_id();

// Reads input from the console (at most 1 KiB).
// Subsequent calls overwrite an internal buffer, so you need to save the
// information before that. Note: Don't free the result of this function, not a leak.
// If you call this multiple times the subsequent calls write in the same buffer
// as the first ones, since it points to the internal Cin buffer.
// If you need to keep old results make sure to clone string or copy elsewhere.
string os_read_from_console_overwrite_previous_call();

struct os_get_env_result {
  string Value;
  bool Success;
};

// Get the value of an environment variable, returns true if found.
// If not found and _silent_ is false, logs warning.
//
// The caller is responsible for freeing the returned string.
//
// @TODO: Cache this, then we would return a string that needn't be freed and
// avoid allocations when calling this function multiple times.
mark_as_leak os_get_env_result os_get_env(string name, bool silent = false);

// Sets a variable (creates if it doesn't exist yet) in 
// the current process' environment
void os_set_env(string name, string value);

// Remove a variable from the current process' environment
void os_remove_env(string name);

// Returns the content stored in the clipboard as a utf8 string.
// The caller is responsible for freeing.
mark_as_leak string os_get_clipboard_content();

// Sets the clipboard content (expects a utf8 string).
void os_set_clipboard_content(string content);

struct platform_common_state {
  static const s64 CONSOLE_BUFFER_SIZE = 1_KiB;

  char CinBuffer[CONSOLE_BUFFER_SIZE];
  char CoutBuffer[CONSOLE_BUFFER_SIZE];
  char CerrBuffer[CONSOLE_BUFFER_SIZE];

  void *CinHandle, *CoutHandle, *CerrHandle;
  mutex CoutMutex, CinMutex;

  string ModuleName;  // Caches the module name (retrieve this with
                      // os_get_current_module())

  string WorkingDir;  // Caches the working dir (query/modify this with
                      // os_get_working_dir(), os_set_working_dir())
  mutex WorkingDirMutex;

  array<string> Argv;
};

// :GlobalStateNoConstructors:
// We create a byte array which is large enough to hold all global variables
// because that avoids the C++ default constructor erasing the state of the
// struct. We initialize it before we call any C++ constructors in the linker
// table (see some stuff we do in exe_main.cpp in lstd/platform/windows_no_crt).
alignas(64) inline byte PlatformCommonState[sizeof(platform_common_state)];

// Short-hand macro for sanity
#define S ((platform_common_state *)&PlatformCommonState[0])

// Defined in memory.h
allocator platform_get_persistent_allocator();
allocator platform_get_temporary_allocator();

#define PERSISTENT platform_get_persistent_allocator()
#define TEMP platform_get_temporary_allocator()

void report_warning_no_allocations(string message);

inline string os_get_current_module() { return S->ModuleName; }

inline array<string> os_get_command_line_arguments() { return S->Argv; }

// This needs to be called when our program runs, but also when a new thread
// starts! See windows_common.cpp for implementation details. Note: You
// shouldn't ever call this.
//
// TODO: Move this from os to init module or sth. doesn't seem like its place is here.
inline void platform_init_context() {
  auto newContext = context(context::dont_init_t{});
  newContext.ThreadID = os_get_current_thread_id();
  newContext.Alloc = {};
  newContext.AllocAlignment = POINTER_SIZE;
  newContext.AllocOptions = 0;
  newContext.LogAllAllocations = false;
  newContext.PanicHandler = default_panic_handler;
  newContext.Log = &cout;
  newContext.FmtDisableAnsiCodes = false;
#if defined DEBUG_MEMORY
  newContext.DebugMemoryHeapVerifyFrequency = 255;
  newContext
      .DebugMemoryPrintListOfUnfreedAllocationsAtThreadExitOrProgramTermination =
      false;
#endif
  newContext.FmtParseErrorHandler = fmt_default_parse_error_handler;
  newContext._HandlingPanic = false;
  newContext._LoggingAnAllocation = false;
  OVERRIDE_CONTEXT(newContext);

  *const_cast<allocator *>(&TemporaryAllocator) = {
      arena_allocator, (void *)&TemporaryAllocatorData};
}

//
// Initializes the state we need to function.
//
inline void platform_init_common_state() {
  memset0(S, sizeof(platform_common_state));

  S->CinMutex = create_mutex();
  S->CoutMutex = create_mutex();
#if defined LSTD_NO_CRT
  S->ExitScheduleMutex = create_mutex();
#endif
  S->WorkingDirMutex = create_mutex();

  void platform_init_allocators();
  platform_init_allocators();

#if defined DEBUG_MEMORY
  debug_memory_init();
#endif

  void platform_specific_init_common_state();
  platform_specific_init_common_state();
}

//
// Reports leaks, uninitializes mutexes.
//
inline void platform_uninit_state() {
#if defined DEBUG_MEMORY
  debug_memory_uninit();
#endif

  // Uninit mutexes
  free_mutex(&S->CinMutex);
  free_mutex(&S->CoutMutex);
#if LSTD_NO_CRT
  free_mutex(&S->ExitScheduleMutex);
#endif
  free_mutex(&S->WorkingDirMutex);

  void platform_uninit_allocators();
  platform_uninit_allocators();
}

//
// This must be called first thing in the
// program (or one of the first to ensure
// proper initialization) before using the library.
//
// When we compile on Windows or on Linux with or without CRT, we
// ensure this gets called before all global constructors.
// This means you can safely use the library in global
// state initialization, if you wish to.
//
inline void platform_state_init() {
  // This prepares the global thread-local
  // immutable Context variable (see context.h)
  LSTD_NAMESPACE::platform_init_context();

  platform_init_common_state();

#if OS == WINDOWS
  void win32_crash_handler_init();
  win32_crash_handler_init();
#endif

  atexit(platform_uninit_state);
}
LSTD_END_NAMESPACE

#if OS == WINDOWS
#include "windows/common.h"
#elif OS == MACOS || OS == LINUX 
#include "posix/common.h"
#elif OS == NO_OS
// No OS (e.g. programming on baremetal).
// Let the user define interfacing with hardware.
#else
#error Implement.
#endif

#undef S
#undef PERSISTENT
#undef TEMP