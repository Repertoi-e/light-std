#include "lstd/os.h"

// The DECLARE_ALTERNATE_NAME macro provides an architecture-neutral way
// of specifying /alternatename comments to the linker.  It prepends the leading
// decoration character for x86 and hybrid and leaves names unmodified for other
// architectures.

#define DECLARE_ALTERNATE_NAME_DATA(name, alternate_name) __pragma(comment(linker, "/alternatename:" #name "=" #alternate_name))

// If we are building a dll that links to this library we don't need a valid "main"
// since it will never get called anyways. This is here as a stub and only called when on an .exe
// and the programmer forgot their entry point.
extern "C" int main_stub(int argc, char *argv[]) {
    debug_break();
    (void) ("Did you forget to add an entry point to your program?");
    return 666;
}

DECLARE_ALTERNATE_NAME_DATA(main, main_stub)

extern "C" int __cdecl _purecall() {
    debug_break();
    LSTD_NAMESPACE::os_abort();
    return 0;
}

// Default definition of _fltused.
extern "C" int _fltused{0x9875};
