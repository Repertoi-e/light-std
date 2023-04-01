#include "lstd/common.h"

#if defined LSTD_NO_CRT

#include "common.h"
#include "lstd/common.h"
#include "lstd/context.h"
#include "lstd/os.h"
#include "lstd/os/windows/api.h"  // For definitions

#if OS != WINDOWS
#error LSTD_NO_CRT is Windows-only
#endif

// Define these as nullptr
extern "C" _CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = {
    nullptr};  // C initializers (first)
extern "C" _CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = {
    nullptr};  // C initializers (last)
extern "C" _CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = {
    nullptr};  // C++ initializers (first)
extern "C" _CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = {
    nullptr};  // C++ initializers (last)
extern "C" _CRTALLOC(".CRT$XPA") _PVFV __xp_a[] = {
    nullptr};  // C pre-terminators (first)
extern "C" _CRTALLOC(".CRT$XPZ") _PVFV __xp_z[] = {
    nullptr};  // C pre-terminators (last)
extern "C" _CRTALLOC(".CRT$XTA") _PVFV __xt_a[] = {
    nullptr};  // C terminators (first)
extern "C" _CRTALLOC(".CRT$XTZ") _PVFV __xt_z[] = {
    nullptr};  // C terminators (last)

// Commented out the stuff we don't care about.
// Turns out we commented the entire function.
// This literally does nothing right know. Oh well.
static int __cdecl pre_c_initialization() {
#if BITS == 32
  // Clear the x87 exception flags.  Any other floating point initialization
  // should already have taken place before this function is called.
  _asm { fnclex}
  _initialize_default_precision();
#endif
  return 0;
}

// int argc, char *argv[] <- we don't pass in these. Use
// os_get_command_line_arguments().
extern "C" int main();

// We call this to init thread local storage.
//
// * Define an initialized callback function pointer, so CRT startup code knows
// * we have dynamically initialized __declspec(thread) variables that need to
// * be initialized at process startup for the primary thread.
extern "C" const PIMAGE_TLS_CALLBACK __dyn_tls_init_callback;

// Defined in tls.cpp.
extern "C" bool __cdecl __scrt_is_nonwritable_in_current_image(
    void const *target);

extern "C" {
void *MainContext;
}

//
// Entry point for executables
//
// Code taken (and slightly modifided) from: __scrt_common_main_seh in
// exe_common.inl from the Visual C++ Source Directories shipped with Visual
// Studio 2019.
//
// I hope this is legal.
extern "C" void main_no_crt() {
  // This initializaton is similar to the CRT initialization that happens before
  // calling the user main function. Actually these happen before calling any
  // C/C++ initialization functions/constructors, because the user code might
  // want to use stuff from the library in e.g. in a constructor of a global
  // variable. Basically all this stuff needs to work before ANY actual
  // programmer code is run.
  //
  // When we link with the CRT (and don't compile all this stub code) we put
  // these in the in linker tables. See e.g. windows_common.cpp

  platform_state_init();

  // These call the tables that the linker has filled with initialization
  // routines for global variables
  if (lstd_initterm_e(__xi_a, __xi_z) != 0) {
    debug_break();
    return;
  }
  lstd_initterm(__xc_a, __xc_z);

  // We do this to avoid reinitializing in __dyn_tls_init (in tlsdyn.cpp)
  MainContext = (void *)&LSTD_NAMESPACE::Context;

  // * If this module has any dynamically initialized __declspec(thread) (thread
  // local) variables,
  // * then we invoke their initialization for the primary thread used to start
  // the process:
  if (*__dyn_tls_init_callback != nullptr &&
      __scrt_is_nonwritable_in_current_image(__dyn_tls_init_callback)) {
    (*__dyn_tls_init_callback)(nullptr, DLL_THREAD_ATTACH, nullptr);
  }

  int mainResult = main();

  // exit does any uninitting we need to do.
  // exit also calls functions scheduled with exit_schedule.
  LSTD_NAMESPACE::exit(mainResult);
}

typedef BOOL(WINAPI *__scrt_dllmain_type)(HINSTANCE, DWORD, LPVOID);

extern "C" BOOL WINAPI DllMain(HINSTANCE const instance, DWORD const reason,
                               LPVOID const reserved);

// The client may define a _pRawDllMain.  This function gets called for attach
// notifications before any other function is called, and gets called for detach
// notifications after any other function is called.  If no _pRawDllMain is
// defined, it is aliased to the no-op _pDefaultRawDllMain.
extern "C" extern __scrt_dllmain_type const _pRawDllMain;

// This flag is incremented each time DLL_PROCESS_ATTACH is processed
// successfully and is decremented each time DLL_PROCESS_DETACH is processed
// (the detach is always assumed to complete successfully).
static int __proc_attached = 0;

static BOOL __cdecl dllmain_crt_process_attach(HMODULE const instance,
                                               LPVOID const reserved) {
  platform_state_init();

  if (lstd_initterm_e(__xi_a, __xi_z) != 0) {
    debug_break();
    return 0;
  }
  lstd_initterm(__xc_a, __xc_z);

  // If we have any dynamically initialized __declspec(thread) variables, we
  // invoke their initialization for the thread on which the DLL is being
  // loaded.  We cannot rely on the OS performing the initialization with the
  // DLL_PROCESS_ATTACH notification because, on Windows Server 2003 and below,
  // that call happens before the CRT is initialized.
  if (*__dyn_tls_init_callback != nullptr &&
      __scrt_is_nonwritable_in_current_image(__dyn_tls_init_callback)) {
    (*__dyn_tls_init_callback)(instance, DLL_THREAD_ATTACH, reserved);
  }

  ++__proc_attached;
  return 1;
}

//
// clang-format off
//
// DLL Uninitialization of the CRT
//
// +----------------------+
// | UserEXE!main returns |
// +--------+-------------+
//          |
// +--------v-----------------------+
// | ExitProcess/LdrShutdownProcess |
// +--------+-----------------------+
//          |
// +--------v--------+
// | UserDLL!DLLMain |
// +--------+--------+
//          |
// +--------v----------------------------------------------+
// | UserDLL!dllmain_crt_process_detach                    |        +-----------------------------+
// |                                              +-----------------> UCRT _cexit()               |
// |  + Startup Lock +-------------------+        |        |        |                             |
// |  |                                  |        |        |        | Run onexit table            |
// |  |  __scrt_dllmain_uninitialize_c() |        |        |        | Run XP* and XT* terminators |
// |  |   /MD: Run onexit table          |        |        |        |                             |
// |  |   /MT: _cexit() +-------------------------+        |        +-----------------------------+
// |  |                                  |                 |
// |  |                                  |                 |        +---------------------------------------------+
// |  |  __scrt_uninitialize_type_info() |                 |    +---> UCRT Uninitializer Order                    |
// |  |                                  |                 |    |   | (__acrt_uninitialize)                       |
// |  |  _RTC_Terminate()                |                 |    |   |                                             |
// |  |   Run RTC terminators            |                 |    |   | Release when terminating:                   |
// |  |                                  |                 |    |   |  _flushall()                                |
// |  +----------------------------------+                 |    |   |                                             |
// |                                                       |    |   | Debug (always), Release unless terminating: |
// |  __scrt_uninitialize_crt()                            |    |   |  uninitialize_c()                           |
// |   /MT: __acrt_uninitialize() + __vcrt_uninitialize() +-----+   |  uninitialize_environment()                 |
// |   /MD: no-op (handled by UCRT/VCRuntime DLL unload)   |    |   |  uninitialize_allocated_memory()            |
// |                                                       |    |   |  uninitialize_allocated_io_buffers()        |
// |  + __finally +--------------------------------------+ |    |   |  report_memory_leaks()                      |
// |  |                                                  | |    |   |  __acrt_uninitialize_command_line()         |
// |  | __scrt_dllmain_uninitialize_critical()           | |    |   |  __acrt_uninitialize_lowio()                |
// |  |  /MT: __acrt_uninitialize_ptd() +----------------------------->__acrt_uninitialize_ptd()                  |
// |  |       __vcrt_uninitialize_ptd() +--------------------+  |   |  uninitialize_vcruntime() (the OS one)      |
// |  |  /MD: no-op                                      | | |  |   |  __acrt_uninitialize_heap()                 |
// |  |       (handled by UCRT/VCRuntime DLL unload)     | | |  |   |  __acrt_uninitialize_locks()                |
// |  |                                                  | | |  |   |  uninitialize_global_state_isolation()      |
// |  | Ensures PTD is released on error                 | | |  |   |                                             |
// |  | so FLS callbacks don't refer to unloaded module  | | |  |   +---------------------------------------------+
// |  |                                                  | | |  |
// |  +--------------------------------------------------+ | |  |   +--------------------------------------+
// |                                                       | |  +---> VCRuntime Uninitializer Order        |
// +-------------------------------------------------------+ |      | (__vcrt_uninitialize)                |
//                                                           |      |                                      |
// +---------------------------------+                       |      | Debug unless terminating:            |
// | /MD Only                        |                       +-------->__vcrt_uninitialize_ptd()           |
// |                                 |                              |  __vcrt_uninitialize_locks()         |
// |  ucrtbase(d)!__acrt_DllMain     |                              |  __vcrt_uninitialize_winapi_thunks() |
// |   __acrt_uninitialize()         |                              |                                      |
// |                                 |                              +--------------------------------------+
// |                                 |
// |  vcruntime140(d)!__vcrt_DllMain |
// |   __vcrt_uninitialize()         |
// |                                 |
// +---------------------------------+
//
// clang-format on

static BOOL __cdecl dllmain_crt_process_detach(bool const is_terminating) {
  // If the attach did not complete successfully, or if the detach was already
  // executed, do not execute the detach:
  if (__proc_attached <= 0) {
    return 0;
  }

  --__proc_attached;

  // :PlatformExitTermination
  LSTD_NAMESPACE::exit_call_scheduled_functions();
  return 1;
}

static BOOL WINAPI dllmain_crt_dispatch(HINSTANCE const instance,
                                        DWORD const reason,
                                        LPVOID const reserved) {
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      return dllmain_crt_process_attach(instance, reserved);
    case DLL_PROCESS_DETACH:
      return dllmain_crt_process_detach(reserved != nullptr);
  }

  return 1;
}

// Define the _CRT_INIT function for compatibility.
extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE const instance, DWORD const reason,
                                 LPVOID const reserved) {
  return dllmain_crt_dispatch(instance, reason, reserved);
}

static BOOL WINAPI dllmain_raw(HINSTANCE const instance, DWORD const reason,
                               LPVOID const reserved) {
  if (!_pRawDllMain) return 1;

  return _pRawDllMain(instance, reason, reserved);
}

static BOOL __cdecl dllmain_dispatch(HINSTANCE const instance,
                                     DWORD const reason,
                                     LPVOID const reserved) {
  // If this is a process detach notification, check that there was a prior
  // process attach notification that was processed successfully.  This is
  // to ensure that we don't detach more times than we attach.
  if (reason == DLL_PROCESS_DETACH && __proc_attached <= 0) {
    return 0;
  }

  BOOL result = 1;

  if (reason == DLL_PROCESS_ATTACH || reason == DLL_THREAD_ATTACH) {
    result = dllmain_raw(instance, reason, reserved);
    if (!result) return 0;

    result = dllmain_crt_dispatch(instance, reason, reserved);
    if (!result) return 0;
  }

  result = DllMain(instance, reason, reserved);

  // If the client DllMain routine failed, unwind the initialization:
  if (reason == DLL_PROCESS_ATTACH && !result) {
    DllMain(instance, DLL_PROCESS_DETACH, reserved);
    dllmain_crt_dispatch(instance, DLL_PROCESS_DETACH, reserved);
    dllmain_raw(instance, DLL_PROCESS_DETACH, reserved);
  }

  if (reason == DLL_PROCESS_DETACH || reason == DLL_THREAD_DETACH) {
    result = dllmain_crt_dispatch(instance, reason, reserved);
    if (!result) return 0;

    result = dllmain_raw(instance, reason, reserved);
    if (!result) return 0;
  }
  return result;
}

//
// Entry point for executables
//
extern "C" BOOL WINAPI main_no_crt_dll(HINSTANCE const instance,
                                       DWORD const reason,
                                       LPVOID const reserved) {
  return dllmain_dispatch(instance, reason, reserved);
}

#endif
