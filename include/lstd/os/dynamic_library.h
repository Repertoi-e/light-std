#pragma once

#include "../array.h"
#include "../common.h"
#include "../string.h"

//
// Simple wrapper around dynamic libraries and getting addresses of procedures.
//

LSTD_BEGIN_NAMESPACE

using dynamic_library = void *;

dynamic_library os_dynamic_library_load(string path);
void *os_dynamic_library_get_symbol(dynamic_library library, const char *name);
void os_dynamic_library_release(dynamic_library library);

LSTD_END_NAMESPACE

#if OS == WINDOWS
#include "windows/dynamic_library.h"
#elif OS == MACOS || OS == LINUX 
#include "posix/dynamic_library.h"
#elif OS == NO_OS
// No OS (e.g. programming on baremetal).
// Let the user define interfacing with hardware.
#else
#error Implement.
#endif