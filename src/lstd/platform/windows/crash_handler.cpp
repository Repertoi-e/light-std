#include "lstd/common.h"

#if OS == WINDOWS

#include "lstd/lstd.h"
#include "lstd/os/windows/api.h"     // Declarations of Win32 functions
#include "lstd/os/windows/common.h"  // Declarations of Win32 functions

LSTD_BEGIN_NAMESPACE

#define CALLSTACK_DEPTH 6

// @TODO: Factor the stack walking part of this function into a
// os_get_call_stack() which can be used anywhere in the program.

static LONG exception_filter(LPEXCEPTION_POINTERS e) {
    u32                     exceptionCode = e->ExceptionRecord->ExceptionCode;
    array<os_function_call> callStack     = os_get_call_stack(0, CALLSTACK_DEPTH, e->ContextRecord);

#define CODE_DESCR(code) \
    if (exceptionCode = code) desc = #code

    const char *desc = null;

    CODE_DESCR(EXCEPTION_ACCESS_VIOLATION);
    else CODE_DESCR(EXCEPTION_ACCESS_VIOLATION); else CODE_DESCR(EXCEPTION_DATATYPE_MISALIGNMENT);
    else CODE_DESCR(EXCEPTION_BREAKPOINT); else CODE_DESCR(EXCEPTION_SINGLE_STEP);
    else CODE_DESCR(EXCEPTION_ARRAY_BOUNDS_EXCEEDED); else CODE_DESCR(EXCEPTION_FLT_DENORMAL_OPERAND);
    else CODE_DESCR(EXCEPTION_FLT_DIVIDE_BY_ZERO); else CODE_DESCR(EXCEPTION_FLT_INEXACT_RESULT);
    else CODE_DESCR(EXCEPTION_FLT_INVALID_OPERATION); else CODE_DESCR(EXCEPTION_FLT_OVERFLOW);
    else CODE_DESCR(EXCEPTION_FLT_STACK_CHECK); else CODE_DESCR(EXCEPTION_FLT_UNDERFLOW);
    else CODE_DESCR(EXCEPTION_INT_DIVIDE_BY_ZERO); else CODE_DESCR(EXCEPTION_INT_OVERFLOW);
    else CODE_DESCR(EXCEPTION_PRIV_INSTRUCTION); else CODE_DESCR(EXCEPTION_IN_PAGE_ERROR);
    else CODE_DESCR(EXCEPTION_ILLEGAL_INSTRUCTION); else CODE_DESCR(EXCEPTION_NONCONTINUABLE_EXCEPTION);
    else CODE_DESCR(EXCEPTION_STACK_OVERFLOW); else CODE_DESCR(EXCEPTION_INVALID_DISPOSITION);
    else CODE_DESCR(EXCEPTION_GUARD_PAGE); else CODE_DESCR(EXCEPTION_INVALID_HANDLE);
    else CODE_DESCR(EXCEPTION_POSSIBLE_DEADLOCK); string message = sprint("{} ({:#x})", desc ? desc : "Unknown exception", exceptionCode);
    defer(free(message));

    Context.PanicHandler(message, callStack);

    For(callStack) { free(it); }
    free(callStack);

    return EXCEPTION_EXECUTE_HANDLER;
}

void win32_crash_handler_init() {
    // We don't support 32 bit

    // auto [processor, success] = os_get_env("PROCESSOR_ARCHITECTURE");
    // assert(success);
    //
    // if (processor == string("EM64T") || processor == string("AMD64")) {
    // } else if (processor == string("x86")) {
    //     MachineType = IMAGE_FILE_MACHINE_I386;
    // }

    // assert(MachineType && "Machine type not supported");

    SetUnhandledExceptionFilter(exception_filter);
}

LSTD_END_NAMESPACE

#endif
