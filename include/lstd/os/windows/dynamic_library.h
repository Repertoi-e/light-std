#pragma once

#include "api.h"  // Declarations of Win32 functions

#include "../../string.h"
#include "memory.h"

LSTD_BEGIN_NAMESPACE

// @Robustness @TODO Check _path_ to be valid
inline dynamic_library os_dynamic_library_load(string path) {
  WIN32_CHECK_BOOL(result, LoadLibraryW(platform_utf8_to_utf16(path)));
  return (dynamic_library)result;
}

// Gets a symbol by an ASCII string
inline void *os_dynamic_library_get_symbol(dynamic_library library,
                                           const char *name) {
  return (void *)GetProcAddress((HMODULE)library, name);
}

// Call when done doing stuff with the dll
inline void os_dynamic_library_release(dynamic_library library) {
  if (library) {
    FreeLibrary((HMODULE)library);
  }
}

LSTD_END_NAMESPACE
