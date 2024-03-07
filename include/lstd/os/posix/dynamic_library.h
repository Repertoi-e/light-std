#pragma once

#include "../../string.h"
#include "memory.h"

#include <dlfcn.h> 

LSTD_BEGIN_NAMESPACE

inline dynamic_library os_dynamic_library_load(string path) {
    // Load the dynamic library using dlopen
    dynamic_library library = dlopen(to_c_string_temp(path), RTLD_NOW);
    if (!library) {
        // Handle error if necessary
        // dlerror() can be used to retrieve more information about the error
    }
    return library;
}

inline void *os_dynamic_library_get_symbol(dynamic_library library, const char *name) {
    // Get the symbol address using dlsym
    void *symbol = dlsym(library, name);
    if (!symbol) {
        // Handle error if necessary
        // dlerror() can be used to retrieve more information about the error
    }
    return symbol;
}

inline void os_dynamic_library_release(dynamic_library library) {
    // Close the dynamic library using dlclose
    if (library) {
        dlclose(library);
    }
}

LSTD_END_NAMESPACE
