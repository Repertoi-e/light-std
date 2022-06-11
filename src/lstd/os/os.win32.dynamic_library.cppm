module;

#include "lstd/platform/windows.h"  // Declarations of Win32 functions

//
// Simple wrapper around dynamic libraries and getting addresses of procedures.
//

export module lstd.os.win32.dynamic_library;

import lstd.string;
import lstd.os.win32.memory;

LSTD_BEGIN_NAMESPACE

export {
    using dynamic_library = void *;

    // We expect _path_ to be valid
    // @TODO: Perhaps we can check
    dynamic_library os_dynamic_library_load(string path);

    // Gets a symbol by an ASCII string
    void *os_dynamic_library_get_symbol(dynamic_library library, const char *name);

    // Call when done doing stuff with the dll
    void release(dynamic_library library);
}

dynamic_library os_dynamic_library_load(string path) {
    WIN32_CHECK_BOOL(result, LoadLibraryW(platform_utf8_to_utf16(path)));
    return (dynamic_library) result;
}

void release(dynamic_library library) {
    if (library) {
        FreeLibrary((HMODULE) library);
    }
}

void *os_dynamic_library_get_symbol(dynamic_library library, const char *name) {
    return (void *) GetProcAddress((HMODULE) library, name);
}

LSTD_END_NAMESPACE
