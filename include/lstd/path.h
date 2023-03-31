#pragma once

#include "common.h"

//
// This module provides facilities to work with paths and files.
// Also provides facilities to query the OS and move/copy/read/write, etc.
// All functions are prefixed with "path_" so you can find them easily with
// autocomplete.
//
// We export platforms-specific modules depending on the platform we are
// compiling on. The rationale is that paths are platform specific and so any
// program input dealing with paths would be different and so we try to do the
// best thing.
//
// For specific cases - in order to work with specific path format you can e.g.
// explicitly include os/windows/path.h
//

#if OS == WINDOWS
#include "os/windows/path.h"
#elif OS == LINUX
#include "os/posix/path.h"
#endif
