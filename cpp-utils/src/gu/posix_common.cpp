#include "common.h"

#if defined OS_LINUX || defined OS_MAC

#include "memory/allocator.h"
#include "string/print.h"

#include <sys/mman.h>
#include <sys/time.h>

#include <termios.h>
#include <unistd.h>

GU_BEGIN_NAMESPACE

void *linux_allocator(Allocator_Mode mode, void *allocatorData, size_t size, void *oldMemory, size_t oldSize,
                      s32 options) {
    switch (mode) {
        case Allocator_Mode::ALLOCATE:
            [[fallthrough]];
        case Allocator_Mode::RESIZE:
            void *result;
            if (mode == Allocator_Mode::ALLOCATE) {
                result = mmap(null, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
            } else {
                result = mremap(oldMemory, oldSize, size, MREMAP_MAYMOVE);
            }
			if (result == (void *) -1) return 0;
            return result;
        case Allocator_Mode::FREE:
            munmap(oldMemory, oldSize);
            return null;
        case Allocator_Mode::FREE_ALL:
            return null;
        default:
            assert(false);  // We shouldn't get here
    }
    return null;
}

Allocator_Func __default_allocator = linux_allocator;

void exit_program(int code) { _exit(code); }

void default_assert_handler(bool failed, const char *file, int line, const char *failedCondition) {
    if (failed) {
        print("\x1b[31m>>> %:%, Assert failed: %\x1b[0m\n", file, line, failedCondition);
        exit_program(-1);
    }
}

void print_string_to_console(string const &str) { write(STDOUT_FILENO, str.Data, str.Size); }

void wait_for_input(b32 message) {
    if (message) print("Press ENTER to continue...\n");
    getchar();
}

f64 get_wallclock_in_seconds() {
    timeval time;
    assert(!gettimeofday(&time, 0));
    return (f64) time.tv_sec + (f64) time.tv_usec * 0.000001;
}

GU_END_NAMESPACE

#endif // defined OS_LINUX || defined OS_MAC