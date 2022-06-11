module;

#include "../common.h"

export module lstd.os;

//
// This module provides general functions related to platform specific tasks.
// We export platforms-specific modules depending on the platform we are compiling on.
//
// Note: To work with files and paths, take a look at the lstd.path module.
//

#if OS == WINDOWS
export import lstd.os.win32.common;
export import lstd.os.win32.memory;
export import lstd.os.win32.dynamic_library;
#else
#endif
