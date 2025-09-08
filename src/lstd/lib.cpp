#include "context.cpp"
#include "memory.cpp"

#include "platform/memory.cpp"

#if OS == WINDOWS
// ..
#elif OS == MACOS || OS == LINUX
#include "platform/posix/thread.cpp"
#endif

#include "vendor/tlsf/tlsf.cpp"

// Unicode and string helpers implementation
#include "string.cpp"

#include "fmt/fmt.cpp"
#include "fmt/write.cpp"
#include "fmt/float_grisu.cpp"
#include "fmt/float_dragonbox.cpp"
