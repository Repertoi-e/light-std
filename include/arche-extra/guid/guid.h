#pragma once

#include "arche/common.h"

#if OS == WINDOWS
#include "os/windows/guid.h"
#elif OS == MACOS || OS == LINUX
#include "os/posix/guid.h"
#elif OS == WASM
#include "os/wasm/guid.h"
#else
#error Implement.
#endif
