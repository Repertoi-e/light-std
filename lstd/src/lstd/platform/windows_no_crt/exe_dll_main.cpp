#include "lstd/common.h"

#if defined LSTD_NO_CRT

#include "common.h"
#include "lstd/common.h"
#include "lstd/platform/windows.h"  // For definitions

import lstd.os;

#if OS != WINDOWS
#error LSTD_NO_CRT is Windows-only
#endif

// Define these as nullptr
extern "C" _CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = {nullptr};  // C initializers (first)
extern "C" _CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = {nullptr};  // C initializers (last)
extern "C" _CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = {nullptr};  // C++ initializers (first)
extern "C" _CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = {nullptr};  // C++ initializers (last)
extern "C" _CRTALLOC(".CRT$XPA") _PVFV __xp_a[] = {nullptr};  // C pre-terminators (first)
extern "C" _CRTALLOC(".CRT$XPZ") _PVFV __xp_z[] = {nullptr};  // C pre-terminators (last)
extern "C" _CRTALLOC(".CRT$XTA") _PVFV __xt_a[] = {nullptr};  // C terminators (first)
extern "C" _CRTALLOC(".CRT$XTZ") _PVFV __xt_z[] = {nullptr};  // C terminators (last)

// Commented out the stuff we don't care about.
// Turns out we commented the entire function.
// This literally does nothing right know. Oh well.
file_scope int __cdecl pre_c_initialization() {
    // CRT stuff:
    // main_policy::set_app_type();

    // file_policy::set_fmode();
    // file_policy::set_commode();

    // This applies to DLLS that use the Universal CRT DLL.
    // if (!__scrt_initialize_onexit_tables(__scrt_module_type::exe))
    //     __scrt_fastfail(FAST_FAIL_FATAL_APP_EXIT);

    // Do we need this? We don't target 32-bit.
#ifdef _M_IX86
    // Clear the x87 exception flags.  Any other floating point initialization
    // should already have taken place before this function is called.
    _asm { fnclex }
#endif

    // No run time checks, please.
    // #ifdef _RTC
    //     _RTC_Initialize();
    //     atexit(_RTC_Terminate);
    // #endif

    // We do this elsewhere. Call os_get_command_line_arguments() to get the == of argv/argc.
    //if (argv_policy::configure_argv() != 0)
    //    __scrt_fastfail(FAST_FAIL_FATAL_APP_EXIT);

    // No, please.
    // __scrt_initialize_type_info();

    // We don't have a handler for floating point exceptions.
    // I'm still comtemplating on what to do in that regard..
    // We will probably add one and control it from the Context.
    //
    // * If the user provided a _matherr handler, register it with the Universal
    // * CRT.  Windows OS components cannot set a custom matherr handler (this is
    // * a policy decision, to reduce complexity).
    // #ifndef _CRT_WINDOWS
    //     if (__scrt_is_user_matherr_present()) {
    //         __setusermatherr(_matherr);
    //     }
    // #endif

    // This is used for "security-enhanced CRT functions".
    // No! Cut the bs.
    // _initialize_invalid_parameter_handler();

    // "In Intel® processors, the flush - to - zero(FTZ) and denormals - are - zero(DAZ) flags in the
    // MXCSR register are used to control floating - point calculations."
    // In MSVC this is controlled via a link option and is normally called here.
    // The function which does the stuff is actually:
    //
    // _controlfp_s(nullptr, _DN_FLUSH, _MCW_DN);
    //
    // I know this is way too buried in the middle of no-where but still I figured I would put it here.

    // Do we need this? We don't target 32-bit.
#ifdef _M_IX86
    _initialize_default_precision();
#endif

    // _configthreadlocale(_get_startup_thread_locale_mode());

    // This caches environment variables from the OS instead of having to query it all the time.
    // Actually.. this might be good. @TODO Cache os_get_env().
    //
    // if (_should_initialize_environment())
    //     environment_policy::initialize_environment();

    // WinRT is some bullshit for Windows Store apps.
    // __scrt_initialize_winrt();

    // * Optionally initializes a process-wide MTA (via CoIncrementMTAUsage).
    // * In Desktop apps, this is conditional upon the exe_initialize_mta.lib linkopt.
    // * In Windows Store apps, enclaves, and DLLs, this has no effect.
    //
    // What is MTA?
    // if (__scrt_initialize_mta() != 0) {
    //     __scrt_fastfail(FAST_FAIL_FATAL_APP_EXIT);
    // }

    return 0;
}

// Note: Comments taken from the Visual C++ source code will be marked with *.
//
// * When both the PGO instrumentation library and the CRT are statically linked,
// * PGO will initialize itself in XIAB. We do most pre-C initialization before
// * PGO is initialized, but defer some initialization steps to after. See the
// * commentary in post_pgo_initialization for details.
//
// What is pogo?
//

// * _CRTALLOC(".CRT$XIAA")
// * static _PIFV pre_c_initializer = pre_c_initialization;
// * _CRTALLOC(".CRT$XIAC")
// * static _PIFV post_pgo_initializer = post_pgo_initialization;

// pre_cpp_initialization does some bullshit with "operator new" handlers.
// * _CRTALLOC(".CRT$XCAA")
// * static _PVFV pre_cpp_initializer = pre_cpp_initialization;

// int argc, char *argv[] <- we don't pass in these. Use os_get_command_line_arguments().
extern "C" int main();

// We call this to init thread local storage.
//
// * Define an initialized callback function pointer, so CRT startup code knows
// * we have dynamically initialized __declspec(thread) variables that need to
// * be initialized at process startup for the primary thread.
extern "C" const PIMAGE_TLS_CALLBACK __dyn_tls_init_callback;

LSTD_BEGIN_NAMESPACE
void win32_crash_handler_init();

// @Cleanup: Decouple lstd lstd-graphics
void win32_monitor_init();
void win32_window_init();
void win32_monitor_uninit();
void win32_window_uninit();
LSTD_END_NAMESPACE

// Defined in tls.cpp.
extern "C" bool __cdecl __scrt_is_nonwritable_in_current_image(void const *target);

// :PlatformStateInit
void platform_state_init() {
    // This prepares the global thread-local immutable Context variable (see lstd.context)
    LSTD_NAMESPACE::platform_init_context();

    LSTD_NAMESPACE::platform_init_global_state();
    LSTD_NAMESPACE::win32_crash_handler_init();
    LSTD_NAMESPACE::win32_monitor_init();
    LSTD_NAMESPACE::win32_window_init();
}

//
// Entry point for executables
//
// Code taken (and slightly modifided) from: __scrt_common_main_seh in exe_common.inl
// from the Visual C++ Source Directories shipped with Visual Studio 2019.
//
// I hope this is legal.
extern "C" void main_no_crt() {
    // This initializaton is similar to the CRT initialization that happens before calling the user main function.
    // Actually these happen before calling any C/C++ initialization functions/constructors,
    // because the user code might want to use stuff from the library in e.g. in a constructor of a global variable.
    // Basically all this stuff needs to work before ANY actual programmer code is run.
    //
    // When we link with the CRT (and don't compile all this stub code) we put these in the in linker tables.
    // See e.g. windows_common.cpp

    platform_state_init();

    // These call the tables that the linker has filled with initialization routines for global variables
    if (lstd_initterm_e(__xi_a, __xi_z) != 0) {
        debug_break();
        return;
    }
    lstd_initterm(__xc_a, __xc_z);

    // * If this module has any dynamically initialized __declspec(thread) (thread local) variables,
    // * then we invoke their initialization for the primary thread used to start the process:
    if (*__dyn_tls_init_callback != nullptr && __scrt_is_nonwritable_in_current_image(__dyn_tls_init_callback)) {
        (*__dyn_tls_init_callback)(nullptr, DLL_THREAD_ATTACH, nullptr);
    }

    // If this module has any thread-local destructors, register the
    // callback function with the Unified CRT to run on exit.
    // _tls_callback_type const *const tls_dtor_callback = __scrt_get_dyn_tls_dtor_callback();
    // if (*tls_dtor_callback != nullptr && __scrt_is_nonwritable_in_current_image(tls_dtor_callback)) {
    //     _register_thread_local_exe_atexit_callback(*tls_dtor_callback);
    // }

    int mainResult = main();

    // Don't manage me, please.
    // if (!__scrt_is_managed_app())
    //     exit(main_result);

    // exit does any uninitting we need to do.
    // exit also calls functions scheduled with exit_schedule.
    LSTD_NAMESPACE::exit(mainResult);
}

typedef BOOL(WINAPI *__scrt_dllmain_type)(HINSTANCE, DWORD, LPVOID);

extern "C" BOOL WINAPI DllMain(HINSTANCE const instance, DWORD const reason, LPVOID const reserved);

// The client may define a _pRawDllMain.  This function gets called for attach
// notifications before any other function is called, and gets called for detach
// notifications after any other function is called.  If no _pRawDllMain is
// defined, it is aliased to the no-op _pDefaultRawDllMain.
extern "C" extern __scrt_dllmain_type const _pRawDllMain;

// * Moved to stubs.cpp
// * extern "C" extern __scrt_dllmain_type const _pDefaultRawDllMain = nullptr;
// * _VCRT_DECLARE_ALTERNATE_NAME_DATA(_pRawDllMain, _pDefaultRawDllMain)

// This flag is incremented each time DLL_PROCESS_ATTACH is processed successfully
// and is decremented each time DLL_PROCESS_DETACH is processed (the detach is
// always assumed to complete successfully).
static int __proc_attached = 0;

static BOOL __cdecl dllmain_crt_process_attach(HMODULE const instance,
                                               LPVOID const reserved) {
    // if (!__scrt_initialize_crt(__scrt_module_type::dll))
    //     return 0;

    // bool const is_nested = __scrt_acquire_startup_lock();
    // bool fail            = true;
    // __try {
    // if (__scrt_current_native_startup_state != __scrt_native_startup_state::uninitialized)
    //     __scrt_fastfail(FAST_FAIL_FATAL_APP_EXIT);

    // __scrt_current_native_startup_state = __scrt_native_startup_state::initializing;

    // if (!__scrt_dllmain_before_initialize_c())
    //     __leave;

    // #ifdef _RTC
    //         _RTC_Initialize();
    // #endif

    // __scrt_initialize_type_info();

    // __scrt_initialize_default_local_stdio_options();

    platform_state_init();

    if (lstd_initterm_e(__xi_a, __xi_z) != 0) {
        debug_break();
        return 0;
    }

    // if (!__scrt_dllmain_after_initialize_c())
    //     __leave;

    lstd_initterm(__xc_a, __xc_z);

    // __scrt_current_native_startup_state = __scrt_native_startup_state::initialized;
    // fail                                = false;
    //} __finally {
    //    __scrt_release_startup_lock(is_nested);
    //}
    //if (fail)
    //    return 0;

    // If we have any dynamically initialized __declspec(thread) variables, we
    // invoke their initialization for the thread on which the DLL is being
    // loaded.  We cannot rely on the OS performing the initialization with the
    // DLL_PROCESS_ATTACH notification because, on Windows Server 2003 and below,
    // that call happens before the CRT is initialized.
    if (*__dyn_tls_init_callback != nullptr && __scrt_is_nonwritable_in_current_image(__dyn_tls_init_callback)) {
        (*__dyn_tls_init_callback)(instance, DLL_THREAD_ATTACH, reserved);
    }

    ++__proc_attached;
    return 1;
}

//  ******************** HOLY FUCKING SHIT, MICROSOFT
//  ******************** HOLY FUCKING SHIT, MICROSOFT
//  ******************** HOLY FUCKING SHIT, MICROSOFT
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
    LSTD_NAMESPACE::win32_monitor_uninit();
    LSTD_NAMESPACE::win32_window_uninit();
    LSTD_NAMESPACE::platform_uninit_state();

    BOOL result = 1;

    //__try {
    //    bool const is_nested = __scrt_acquire_startup_lock();
    //    __try {
    //        if (__scrt_current_native_startup_state != __scrt_native_startup_state::initialized) {
    //            __scrt_fastfail(FAST_FAIL_FATAL_APP_EXIT);
    //        }
    //
    //        __scrt_dllmain_uninitialize_c();
    //
    //        __scrt_uninitialize_type_info();
    //
    //#ifdef _RTC
    //            _RTC_Terminate();
    //#endif

    //        __scrt_current_native_startup_state = __scrt_native_startup_state::uninitialized;
    //    } __finally {
    //        __scrt_release_startup_lock(is_nested);
    //    }
    //
    //    if (!__scrt_uninitialize_crt(is_terminating, false)) {
    //        result = 0;
    //        __leave;
    //    }
    //} __finally {
    //    __scrt_dllmain_uninitialize_critical();
    //}
    return result;
}

static BOOL WINAPI dllmain_crt_dispatch(
    HINSTANCE const instance,
    DWORD const reason,
    LPVOID const reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            return dllmain_crt_process_attach(instance, reserved);
        case DLL_PROCESS_DETACH:
            return dllmain_crt_process_detach(reserved != nullptr);
            // case DLL_THREAD_ATTACH:
            //     return __scrt_dllmain_crt_thread_attach();
            // case DLL_THREAD_DETACH:
            //     return __scrt_dllmain_crt_thread_detach();
    }

    return 1;
}

// Define the _CRT_INIT function for compatibility.
extern "C" BOOL WINAPI _CRT_INIT(
    HINSTANCE const instance,
    DWORD const reason,
    LPVOID const reserved) {
    return dllmain_crt_dispatch(instance, reason, reserved);
}

static BOOL WINAPI dllmain_raw(
    HINSTANCE const instance,
    DWORD const reason,
    LPVOID const reserved) {
    if (!_pRawDllMain)
        return 1;

    return _pRawDllMain(instance, reason, reserved);
}

static BOOL __cdecl dllmain_dispatch(
    HINSTANCE const instance,
    DWORD const reason,
    LPVOID const reserved) {
    // If this is a process detach notification, check that there was a prior
    // process attach notification that was processed successfully.  This is
    // to ensure that we don't detach more times than we attach.
    if (reason == DLL_PROCESS_DETACH && __proc_attached <= 0) {
        return 0;
    }

    BOOL result = 1;

    //__try {
    if (reason == DLL_PROCESS_ATTACH || reason == DLL_THREAD_ATTACH) {
        result = dllmain_raw(instance, reason, reserved);
        if (!result)
            //__leave;
            return 0;

        result = dllmain_crt_dispatch(instance, reason, reserved);
        if (!result)
            //__leave;
            return 0;
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
        if (!result)
            //__leave;
            return 0;

        result = dllmain_raw(instance, reason, reserved);
        if (!result)
            //__leave;
            return 0;
    }
    // } __except (__scrt_dllmain_exception_filter(
    //     instance,
    //     reason,
    //     reserved,
    //     dllmain_crt_dispatch,
    //     GetExceptionCode(),
    //     GetExceptionInformation())) {
    //     result = 0;
    // }

    return result;
}

//
// Entry point for executables
//
// Code taken (and slightly modifided) from: dll_dllmain.cpp
// from the Visual C++ Source Directories shipped with Visual Studio 2019.
//
// I hope this is legal.
extern "C" BOOL WINAPI main_no_crt_dll(HINSTANCE const instance, DWORD const reason, LPVOID const reserved) {
    // if (reason == DLL_PROCESS_ATTACH) {
    //     // The /GS security cookie must be initialized before any exception
    //     // handling targeting the current image is registered.  No function
    //     // using exception handling can be called in the current image until
    //     // after this call:
    //     __security_init_cookie();
    // }

    return dllmain_dispatch(instance, reason, reserved);
}

#endif
