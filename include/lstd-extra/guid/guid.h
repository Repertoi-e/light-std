#pragma once

#include "lstd/common.h"

#if OS == WINDOWS
#include "os/windows/guid.h"
#elif OS == MACOS || OS == LINUX
#include "os/posix/guid.h"
#else
#error Implement.
#endif
