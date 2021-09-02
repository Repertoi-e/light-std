module;

#include "lstd/memory/string.h"
#include "lstd/types/windows.h"  // Declarations of Win32 functions

//
// Simple wrapper around dynamic libraries and getting addresses of procedures.
//

export module os.win64.dynamic_library;

LSTD_BEGIN_NAMESPACE

// @Cleanup
extern "C" {
wchar_t *utf8_to_utf16(const string &str, allocator alloc);
}

wchar_t *utf8_to_utf16_temp(const string &str) { return utf8_to_utf16(str, {}); }

export {
    struct dynamic_library_t {
    };

    using dynamic_library = dynamic_library_t *;

    dynamic_library os_dynamic_library_load(const string &path) {
        return (dynamic_library) LoadLibraryW(utf8_to_utf16_temp(path));
    }

    // :OverloadFree: We follow the convention to overload the "free" function
    // as a standard way to release resources (may not be just memory blocks).
    void free(dynamic_library library) {
        if (library) {
            FreeLibrary((HMODULE) library);
        }
    }

    void *os_dynamic_library_get_symbol(dynamic_library library, const char *name) {
        return (void *) GetProcAddress((HMODULE) library, name);
    }
}

LSTD_END_NAMESPACE
