#pragma once

#include "memory/string.h"

LSTD_BEGIN_NAMESPACE

// @AvoidInclude
template <typename T>
struct array;

///
/// This file includes general functions related to platform specific tasks - implemented in platform files accordingly
///

//
// @TODO: Have a memory heap API for creating new heaps and choosing which one is used for allocations.
// Also by default we should have different heaps for small/medium/large objects to prevent memory fragmentation.
// Right now we rely on the user being thoughtful about memory management and not calling new/delete left and right.
//

// Allocates memory by calling OS functions
[[nodiscard("Leak")]] void *os_allocate_block(s64 size);

// Allocates one giant block determined from the size of types passed in.
// Returns pointers in the block spaced out accordingly.
// After that this calls constructors on non-scalar values.
//
// Also reserves space for blocks whose size is determined at runtime.
// These get passed in as parameters to the function.
//
// Note: @Robustness This doesn't call constructors on arrays, e.g. my_data_t[n].
//       We can implement this but the code is going to get much more complicated.
//
// This is a utility which aids in reducing fragmentation when you can allocate program state in one place in the code.
template <typename... Types>
[[nodiscard("Leak")]] auto os_allocate_packed(array_view<s64> runtimeSizes) {
    constexpr s64 TYPE_SIZE[] = {sizeof(Types)...};
    constexpr s64 TOTAL_TYPE_SIZE = (sizeof(Types) + ...);

    // We decay, remove pointers and add a pointer in order to handle arrays
    // e.g.  byte[10] -> decays to -> byte *, but here if we add a pointer again, we would get byte **
    // The reason we return pointers is because we return the address in the block each element begins.
    using result_t = tuple<types::add_pointer_t<types::remove_pointer_t<types::decay_t<Types>>>..., void *>;
    result_t result;

    s64 size = TOTAL_TYPE_SIZE;
    For(runtimeSizes) size += it;

    void *block = os_allocate_block(size);

    s64 offset = 0;
    static_for<0, sizeof...(Types)>([&](auto i) {
        using element_t = tuple_get_t<i, result_t>;

        auto *p = (element_t)((byte *) block + offset);
        tuple_get<i>(result) = p;

        using element_t_no_pointer = types::remove_pointer_t<element_t>;

        // Call constructor on values that are not scalars
        // @Robustness This doesn't call constructors on arrays, e.g. my_data_t[n]
        if constexpr (!types::is_scalar<element_t_no_pointer>) {
            new (p) element_t_no_pointer;
        }

        offset += TYPE_SIZE[i];
    });

    tuple_get<sizeof...(Types)>(result) = (byte *) block + offset;

    return result;
}

// Expands/shrinks a memory block allocated by _os_alloc()_.
// This is NOT realloc. When this fails it returns null instead of allocating a new block and copying the contents of the old one.
// That's why it's not called realloc.
[[nodiscard("Leak")]] void *os_resize_block(void *ptr, s64 newSize);

// Returns the size of a memory block allocated by _os_alloc()_ in bytes
s64 os_get_block_size(void *ptr);

// Frees a memory block allocated by _os_alloc()_
void os_free_block(void *ptr);

// Creates/opens a shared memory block and writes data to it (use this for communication between processes)
void os_write_shared_block(const string &name, void *data, s64 size);

// Read data from a shared memory block (use this for communication between processes)
void os_read_shared_block(const string &name, void *out, s64 size);

// Exits the application with the given exit code.
// Also runs all callbacks registered with _exit_schedule()_.
// @TODO: Rename this to just _exit_
void os_exit(s32 exitCode = 0);

// Produces an abnormal program termination.
// Doesn't call any callbacks registered with _exit_schedule()_.
// @TODO: Rename this to just _abort_
void os_abort();

// Returns a time stamp that can be used for time-interval measurements
time_t os_get_time();

// Converts a time stamp acquired by _os_get_time()_ to seconds
f64 os_time_to_seconds(time_t time);

[[nodiscard("Leak")]] string os_get_clipboard_content();
void os_set_clipboard_content(const string &content);

// Returns the path of the current executable or dynamic library (full dir + name).
//
// Don't free the result of this function. This library follows the convention that if the function is not marked as [[nodiscard]], the returned value should not be freed.
string os_get_current_module();

// Returns the current directory of the current process.
// [Windows] The docs say that SetCurrentDirectory/GetCurrentDirectory
//           are not thread-safe but we use a lock so these are.
//
// Don't free the result of this function. This library follows the convention that if the function is not marked as [[nodiscard]], the returned value should not be freed.
string os_get_working_dir();

// Sets the current directory of the current process (needs to be absolute).
// [Windows] The docs say that SetCurrentDirectory/GetCurrentDirectory
//           are not thread-safe but we use a lock so these are.
void os_set_working_dir(const string &dir);

struct os_get_env_result {
    string Value;
    bool Success;
};

// Get the value of an environment variable, returns true if found.
// If not found and silent is false, logs warning.
// The caller is responsible for freeing the string in the returned value.
[[nodiscard("Leak")]] os_get_env_result os_get_env(const string &name, bool silent = false);

// Sets a variable (creates if it doesn't exist yet) in this process' environment
void os_set_env(const string &name, const string &value);

// Delete a variable from the current process' environment
void os_remove_env(const string &name);

// Get a list of parsed command line arguments excluding the first one.
// Normally the first one is the exe name - you can get that with os_get_current_module().
//
// Don't free the result of this function. This library follows the convention that if the function is not marked as [[nodiscard]], the returned value should not be freed.
array<string> os_get_command_line_arguments();

// Returns an ID which uniquely identifies the current process on the system
u32 os_get_pid();

// Reads input from the console (at most 1 KiB).
// Subsequent calls overwrite an internal buffer, so you need to save the information before that.
//
// Don't free the result of this function. This library follows the convention that if the function is not marked as [[nodiscard]], the returned value should not be freed.
bytes os_read_from_console();

// Utility to report hresult errors produces by calling windows functions.
// Shouldn't be used on other platforms
#if OS == WINDOWS

// Logs a formatted error message.
void windows_report_hresult_error(long hresult, const string &call, source_location loc = source_location::current());

// CHECKHR checks the return value of _call_ and if the returned HRESULT is less than zero, reports an error.
#define WIN32_CHECKHR(call)                                          \
    {                                                                \
        long result = call;                                          \
        if (result < 0) windows_report_hresult_error(result, #call); \
    }

// CHECKHR_BOOL checks the return value of _call_ and if the returned is false, reports an error.
#define WIN32_CHECKBOOL(call)                                                                 \
    {                                                                                         \
        bool result = call;                                                                   \
        if (!result) windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), #call); \
    }

// DX_CHECK is used for checking directx calls. The difference from WIN32_CHECKHR is that
// in Release configuration, the macro expands to just the call (no error checking).
#if not defined NDEBUG
#define DX_CHECK(call) WIN32_CHECKHR(call)
#else
#define DX_CHECK(call) call
#endif

#define COM_SAFE_RELEASE(x) \
    if (x) {                \
        x->Release();       \
        x = null;           \
    }

#endif  // OS == WINDOWS

LSTD_END_NAMESPACE
