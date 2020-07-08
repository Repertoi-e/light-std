#pragma once

#include "types.h"

LSTD_BEGIN_NAMESPACE

// @Avoid#include
template <typename T>
struct array;

// @Avoid#include
struct string;

/// General functions related to platform specific tasks - implemented in platform files accordingly

//
// @TODO: Have a memory heap API for creating new heaps and choosing which one is used for allocations.
// Also by default we should have different heaps for small/medium/large objects to prevent memory fragmentation.
// Right now we rely on the user being thoughtful about memory management and not calling new/delete left and right.
//

// Allocates memory by calling OS functions
void *os_allocate_block(s64 size);

// Expands/shrinks a memory block allocated by _os_alloc()_
// This is NOT realloc in the general sense where when this fails it returns null instead of allocating a new block.
// That's why it's not called realloc.
void *os_resize_block(void *ptr, s64 newSize);

// Returns the size of a memory block allocated by _os_alloc()_ in bytes.
s64 os_get_block_size(void *ptr);

// Frees a memory block allocated by _os_alloc()_
void os_free_block(void *ptr);

// Creates/opens a shared memory block and writes data to it (use this for communication between processes)
void os_write_shared_block(string name, void *data, s64 size);

// Read data from a shared memory block (use this for communication between processes)
void os_read_shared_block(string name, void *out, s64 size);

// Exits the application with the given exit code

// @TODO: Have a "at_exit" function which adds callbacks that are called when the program exits (very useful when
// handling resources and we ensure if the program crashes for some reason we don't block them!).

void os_exit(s32 exitCode = 0);

// Returns a time stamp that can be used for time-interval measurements
time_t os_get_time();

// Converts a time stamp acquired by _os_get_time()_ to seconds
f64 os_time_to_seconds(time_t time);

string os_get_clipboard_content();
void os_set_clipboard_content(string content);

// Sleep for _ms_ milliseconds
// void os_sleep(f64 ms);

// Returns the path of the current executable or dynamic library (full dir + name)
string os_get_current_module();

// Returns the current directory of the current process.
// [Windows] The docs say that SetCurrentDirectory/GetCurrentDirectory
//           are not thread-safe but we use a lock so these are.
string os_get_working_dir();

// Sets the current directory of the current process (needs to be absolute).
// [Windows] The docs say that SetCurrentDirectory/GetCurrentDirectory
//           are not thread-safe but we use a lock so these are.
void os_set_working_dir(string dir);

// Get the value of an environment variable, returns true if found.
// If not found and silent is false, logs error to cerr.
bool os_get_env(string *out, string name, bool silent = false);

// Sets a variable (creates if it doesn't exist yet) in this process' environment
void os_set_env(string name, string value);

// Delete a variable from the current process' environment
void os_remove_env(string name);

// Get a list of parsed command line arguments excluding the first one.
// Normally the first one is the exe name - you can get that with os_get_current_module().
array<string> os_get_command_line_arguments();

// Returns an ID which uniquely identifies the current process on the system
u32 os_get_pid();

// Utility to report hresult errors produces by calling windows functions.
// Shouldn't be used on other platforms
#if OS == WINDOWS

// Logs a formatted error message.
void windows_report_hresult_error(long hresult, string call, string file, s32 line);

// CHECKHR checks the return value of _call_ and if the returned HRESULT is less than zero, reports an error.
#define CHECKHR(call)                                                                    \
    {                                                                                    \
        long result = call;                                                              \
        if (result < 0) windows_report_hresult_error(result, #call, __FILE__, __LINE__); \
    }

// DXCHECK is used for checking e.g. directx calls. The difference is that
// in Dist configuration, the macro expands to just the call (no error checking)
// in order to save on performance.
#if defined DEBUG || defined RELEASE
#define DXCHECK(call) CHECKHR(call)
#else
#define DXCHECK(call) call
#endif

#define SAFE_RELEASE(x) \
    if (x) {            \
        x->Release();   \
        x = null;       \
    }

#endif  // OS == WINDOWS

LSTD_END_NAMESPACE
