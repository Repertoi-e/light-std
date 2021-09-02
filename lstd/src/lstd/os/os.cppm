module;

#include "lstd/common/context.h"

export module os;

//
// This module provides general functions related to platform specific tasks.
// All functions are prefixed with "os_" so you can find them easily with autocomplete.
// We export platforms-specific modules depending on the platform we are compiling on.
//
// Note: To work with files, take a look at the path module.
//

#if OS == WINDOWS
export import os.win64.common;
export import os.win64.memory;
export import os.win64.dynamic_library;
#else
#error Implement.
#endif

