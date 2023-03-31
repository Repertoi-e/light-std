#pragma once

#include "common.h"

//
// This module provides general functions related to platform specific tasks.
// We export platforms-specific modules depending on the platform we are
// compiling on.
//
// Note: To work with files and paths, take a look at path.h
//

//
// At the moment, we do not have a common interface for calling functions across
// all operating systems, as different OSs may have varying levels of
// functionality. Instead, each OS header exports its own set of functions, some
// of which may have the same name across platforms (such as
// 'os_allocate_block()' or 'os_dynamic_library_load()' for Windows, Linux,
// etc.). While this approach provides compatibility, we do not define a hard
// abstraction layer yet, as it is not yet clear which platform features are
// universal and which are not. Also I'd be confusing to export all the symbols,
// but be forced for some platform to just assert() in the implementation body.
//
// If we do decide to implement a defined interface in the future, we are not
// yet sure what it will look like."
//

#if OS == WINDOWS
#include "os/windows/common.h"
#include "os/windows/memory.h"
#include "os/windows/dynamic_library.h"
#include "os/windows/thread.h"
#else if OS == NO_OS
// No OS (e.g. programming on baremetal).
// Let the user define interfacing with hardware.
#endif
