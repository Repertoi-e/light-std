#include "memory.cpp"

#include "platform/memory.cpp"

#if OS == WINDOWS
#include "platform/windows/callstack.cpp"
#include "platform/windows/thread.cpp"
#elif OS == MACOS || OS == LINUX
#include "platform/posix/callstack.cpp"
#include "platform/posix/thread.cpp"
#elif OS == WASM
#include "platform/posix/thread.cpp"
#else
#error Implement.
#endif

#include "vendor/tlsf/tlsf.cpp"

#include "string.cpp"
#include "context.cpp"

#include "fmt/float_dragonbox.cpp"
#include "fmt/float_grisu.cpp"
#include "fmt/fmt.cpp"
#include "fmt/write.cpp"

