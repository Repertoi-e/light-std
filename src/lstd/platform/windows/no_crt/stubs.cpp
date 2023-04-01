#include "lstd/os.h"
#include "lstd/os/windows/api.h"  // For definitions

// The DECLARE_ALTERNATE_NAME macro provides an architecture-neutral way
// of specifying /alternatename comments to the linker.  It prepends the leading
// decoration character for x86 and hybrid and leaves names unmodified for other
// architectures.

#define DECLARE_ALTERNATE_NAME_DATA(name, alternate_name) \
  __pragma(comment(linker, "/alternatename:" #name "=" #alternate_name))

// If we are building a dll that links to this library we don't need a valid
// "main" since it will never get called anyways. This is here as a stub and
// only called when on an .exe and the programmer forgot their entry point.
extern "C" int main_stub(int argc, char *argv[]) {
  debug_break();
  (void)"Did you forget to add an entry point to your program?";
  return 666;
}

DECLARE_ALTERNATE_NAME_DATA(main, main_stub)

// Dummy if no DllMain was provided.
extern "C" BOOL WINAPI DllMain_stub(HINSTANCE const instance,
                                    DWORD const reason, LPVOID const reserved) {
  return 1;
}
DECLARE_ALTERNATE_NAME_DATA(DllMain, DllMain_stub)

typedef BOOL(WINAPI *__scrt_dllmain_type)(HINSTANCE, DWORD, LPVOID);

// The client may define a _pRawDllMain.  This function gets called for attach
// notifications before any other function is called, and gets called for detach
// notifications after any other function is called.  If no _pRawDllMain is
// defined, it is aliased to the no-op _pDefaultRawDllMain.
extern "C" extern __scrt_dllmain_type const _pDefaultRawDllMain = nullptr;
DECLARE_ALTERNATE_NAME_DATA(_pRawDllMain, _pDefaultRawDllMain)

extern "C" int __cdecl _purecall() {
  debug_break();
  LSTD_NAMESPACE::abort();
  return 0;
}

// Default definition of _fltused.
extern "C" int _fltused{0x9875};
