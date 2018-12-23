#include "common.h"

#if defined OS_WINDOWS

#if defined COMPILER_MSVC && defined CPPU_NO_CRT
extern "C" {
int _fltused;
}
#endif

#include "io/writer.h"

#include "format/fmt.h"
#include "memory/allocator.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

CPPU_BEGIN_NAMESPACE

void *windows_allocator(Allocator_Mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, s32) {
    switch (mode) {
        case Allocator_Mode::ALLOCATE:
            return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
        case Allocator_Mode::RESIZE:
            return HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, oldMemory, size);
        case Allocator_Mode::FREE:
            HeapFree(GetProcessHeap(), 0, oldMemory);
            return null;
        case Allocator_Mode::FREE_ALL:
            return null;
        default:
            assert(false);  // We shouldn't get here
    }
    return null;
}

Allocator_Func __default_allocator = windows_allocator;

void exit_program(int code) { _exit(code); }

void default_assert_failed(const char *file, int line, const char *condition) {
    fmt::print("{}>>> {}:{}, Assert failed: {}{}\n", fmt::FG::Red, file, line, condition, fmt::FG::Reset);
#if defined COMPILER_MSVC
    __debugbreak();
#else
    exit_program(-1);
#endif
}

Writer &Console_Writer::write(const string_view &str) {
    if (!PlatformData) {
        PlatformData = (size_t) GetStdHandle(STD_OUTPUT_HANDLE);
        if (!SetConsoleOutputCP(CP_UTF8)) {
            fmt::print(">>> Warning, couldn't set console code page to UTF-8. Some characters might be messed up.");
        }
        DWORD dw = 0;
        GetConsoleMode((HANDLE) PlatformData, &dw);
        SetConsoleMode((HANDLE) PlatformData, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    DWORD ignored;
    WriteFile((HANDLE) PlatformData, str.Data, (DWORD) str.ByteLength, &ignored, null);

    return *this;
}

void wait_for_input(b32 message) {
    if (message) fmt::print("Press ENTER to continue...\n");
    getchar();
}

f64 get_wallclock_in_seconds() {
    // #TODO: Not implemented
    // #TODO: Not implemented
    // #TODO: Not implemented
    return 0;
    // #TODO: Not implemented
    // #TODO: Not implemented
    // #TODO: Not implemented

    // timeval time;
    // assert(!gettimeofday(&time, 0));
    // return (f64) time.tv_sec + (f64) time.tv_usec * 0.000001;
}

CPPU_END_NAMESPACE

#endif  // defined OS_WINDOWS