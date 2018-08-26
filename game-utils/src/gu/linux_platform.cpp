#include "common.h"
#include "memory/allocator.h"
#include "string/print.h"

#include <sys/mman.h>
#include <sys/time.h>

#include <unistd.h>
#include <termios.h>

void* linux_allocator(Allocator_Mode mode, void *allocatorData, size_t size, void *oldMemory, size_t oldSize, s32 options) {
    switch (mode) {
    case Allocator_Mode::ALLOCATE:
        [[fallthrough]];
    case Allocator_Mode::RESIZE:
        void *result;
        if (mode == Allocator_Mode::ALLOCATE) {
            result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        } else {
            result = mremap(oldMemory, oldSize, size, MREMAP_MAYMOVE);
        }
        return result;
    case Allocator_Mode::FREE:
        munmap(oldMemory, oldSize);
        return 0;
    case Allocator_Mode::FREE_ALL:
        return 0;
    default:
        assert(false); // We shouldn't get here
    }
    return 0;
}

Allocator_Func __default_allocator = linux_allocator;

void exit_program(int code) {
    _exit(code);
}

void default_failed_assert(const char *file, int line, const char *failedCondition) {
    print(">> %:%, Assertion \"%\" failed.\n", file, line, failedCondition);
    
    exit_program(0);
}

void print_string_to_console(string const &str) {
    write(STDOUT_FILENO, str.Data, str.Size);
}

void wait_for_input(b32 message) {
    if (message) {
        print("Press ENTER to continue...\n");
    }
    getchar();
}

f64 get_wallclock_in_seconds() {
    timeval time;
    assert(!gettimeofday(&time, 0));
    return (f64) time.tv_sec + (f64) time.tv_usec * 0.000001;
}
