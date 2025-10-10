#include "context.cpp"
#include "memory.cpp"

#include "platform/memory.cpp"

#if OS == WINDOWS
#include "platform/windows/thread.cpp"
#include "platform/windows/callstack.cpp"
#elif OS == MACOS || OS == LINUX 
#include "platform/posix/thread.cpp"
#include "platform/posix/callstack.cpp"
#elif OS == WASM
#include "platform/posix/thread.cpp"
#else 
#error Implement.
#endif

#include "vendor/tlsf/tlsf.cpp"

// Unicode and string helpers implementation
#include "string.cpp"
#include "clap.cpp"

#include "fmt/fmt.cpp"
#include "fmt/write.cpp"
#include "fmt/float_grisu.cpp"
#include "fmt/float_dragonbox.cpp"
