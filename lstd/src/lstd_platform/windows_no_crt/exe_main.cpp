#include "lstd/internal/common.h"

#if defined LSTD_NO_CRT

#include "common.h"
#include "lstd/internal/context.h"
#include "lstd/os.h"
#include "lstd/types/windows.h"  // For definitions

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
extern void win32_common_init_global_state();
extern void win32_common_init_context();
extern void win32_crash_handler_init();
LSTD_END_NAMESPACE

// We need to reinit the context after the TLS initalizer fires and resets our state.. sigh.
// We can't just do it once because global variables might still use the context and TLS fires a bit later.
s32 tls_init() {
    LSTD_NAMESPACE::win32_common_init_context();
    return 0;
}

#pragma const_seg(".CRT$XDU")
__declspec(allocate(".CRT$XDU")) _PIFV g_TLSInit = tls_init;
#pragma const_seg()


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
    // because the user code might want to use stuff from the library in e.g. a constructor of a global variable.
    // Basically all this stuff needs to work before ANY actual programmer code is run.
    //
    // We can put these in the beginning of the linker tables (CRT does this), but why bother?
    // This also needs to happen for DLLs.
    //
    LSTD_NAMESPACE::win32_common_init_context();  // This prepares the global thread-local immutable Context variable (see "lstd/internal/context.h")
    LSTD_NAMESPACE::win32_common_init_global_state();
    LSTD_NAMESPACE::win32_crash_handler_init();

    // These call the tables that the linker has filled with initialization routines for global variables
    if (lstd_initterm_e(__xi_a, __xi_z) != 0) {
        debug_break();
        return;
    }
    lstd_initterm(__xc_a, __xc_z);

    // Defined in tls.cpp.
    extern bool __cdecl __scrt_is_nonwritable_in_current_image(void const *const target);

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

    // os_exit does any uninitting we need to do.
    // os_exit also calls functions scheduled with exit_schedule.
    LSTD_NAMESPACE::os_exit(mainResult);
}

#endif
