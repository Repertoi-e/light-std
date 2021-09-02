#include "lstd/common/common.h"

#if OS == WINDOWS
#include "lstd/common/os_function_call.h"
#include "lstd/io.h"
#include "lstd/memory/hash_table.h"
#include "lstd/common/windows.h"  // Declarations of Win32 functions

import fmt;
import os;

LSTD_BEGIN_NAMESPACE

#define CALLSTACK_DEPTH 6

file_scope DWORD MachineType;

// @TODO: Factor the stack walking part of this function into a os_get_call_stack() which can be used anywhere in the program.

file_scope LONG exception_filter(LPEXCEPTION_POINTERS e) {
    u32 exceptionCode = e->ExceptionRecord->ExceptionCode;

    HANDLE hProcess = INVALID_HANDLE_VALUE;
    defer(SymCleanup(hProcess));

    if (!SymInitialize(GetCurrentProcess(), null, true)) return EXCEPTION_EXECUTE_HANDLER;

    auto c = e->ContextRecord;

    STACKFRAME64 sf;
    fill_memory(&sf, 0, sizeof(STACKFRAME64));

    sf.AddrPC.Offset    = c->Rip;
    sf.AddrStack.Offset = c->Rsp;
    sf.AddrFrame.Offset = c->Rbp;
    sf.AddrPC.Mode      = AddrModeFlat;
    sf.AddrStack.Mode   = AddrModeFlat;
    sf.AddrFrame.Mode   = AddrModeFlat;

    array<os_function_call> callStack;

    while (StackWalk64(MachineType, GetCurrentProcess(), GetCurrentThread(), &sf, c, 0, SymFunctionTableAccess64, SymGetModuleBase64, null)) {
        if (sf.AddrFrame.Offset == 0 || callStack.Count >= CALLSTACK_DEPTH) break;

        constexpr auto s = (sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64);
        ULONG64 symbolBuffer[s];

        PSYMBOL_INFO symbol  = (PSYMBOL_INFO) symbolBuffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen   = MAX_SYM_NAME;

        os_function_call call;

        DWORD64 symDisplacement = 0;
        if (SymFromAddr(hProcess, sf.AddrPC.Offset, &symDisplacement, symbol)) {
            call.Name = string(symbol->Name);
            if (call.Name.Length == 0) {
                free(call.Name);
                call.Name = "UnknownFunction";
            }
        }

        IMAGEHLP_LINEW64 lineInfo = {sizeof(IMAGEHLP_LINEW64)};

        DWORD lineDisplacement = 0;
        if (SymGetLineFromAddrW64(hProcess, sf.AddrPC.Offset, &lineDisplacement, &lineInfo)) {
            call.File = internal::platform_utf16_to_utf8(lineInfo.FileName, internal::platform_get_persistent_allocator());
            if (call.File.Length == 0) {
                free(call.File);
                call.File = "UnknownFile";
            }
            call.LineNumber = lineInfo.LineNumber;
        }

        array_append(callStack, call);
    }

#define CODE_DESCR(code) \
    if (exceptionCode = code) desc = #code

    const char *desc = null;

    CODE_DESCR(EXCEPTION_ACCESS_VIOLATION);
    else
        CODE_DESCR(EXCEPTION_ACCESS_VIOLATION);
        else
            CODE_DESCR(EXCEPTION_DATATYPE_MISALIGNMENT);
            else
                CODE_DESCR(EXCEPTION_BREAKPOINT);
                else
                    CODE_DESCR(EXCEPTION_SINGLE_STEP);
                    else
                        CODE_DESCR(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
                        else
                            CODE_DESCR(EXCEPTION_FLT_DENORMAL_OPERAND);
                            else
                                CODE_DESCR(EXCEPTION_FLT_DIVIDE_BY_ZERO);
                                else
                                    CODE_DESCR(EXCEPTION_FLT_INEXACT_RESULT);
                                    else
                                        CODE_DESCR(EXCEPTION_FLT_INVALID_OPERATION);
                                        else
                                            CODE_DESCR(EXCEPTION_FLT_OVERFLOW);
                                            else
                                                CODE_DESCR(EXCEPTION_FLT_STACK_CHECK);
                                                else
                                                    CODE_DESCR(EXCEPTION_FLT_UNDERFLOW);
                                                    else
                                                        CODE_DESCR(EXCEPTION_INT_DIVIDE_BY_ZERO);
                                                        else
                                                            CODE_DESCR(EXCEPTION_INT_OVERFLOW);
                                                            else
                                                                CODE_DESCR(EXCEPTION_PRIV_INSTRUCTION);
                                                                else
                                                                    CODE_DESCR(EXCEPTION_IN_PAGE_ERROR);
                                                                    else
                                                                        CODE_DESCR(EXCEPTION_ILLEGAL_INSTRUCTION);
                                                                        else
                                                                            CODE_DESCR(EXCEPTION_NONCONTINUABLE_EXCEPTION);
                                                                            else
                                                                                CODE_DESCR(EXCEPTION_STACK_OVERFLOW);
                                                                                else
                                                                                    CODE_DESCR(EXCEPTION_INVALID_DISPOSITION);
                                                                                    else
                                                                                        CODE_DESCR(EXCEPTION_GUARD_PAGE);
                                                                                        else
                                                                                            CODE_DESCR(EXCEPTION_INVALID_HANDLE);
                                                                                            else
                                                                                                CODE_DESCR(EXCEPTION_POSSIBLE_DEADLOCK);

    string message = sprint("{} ({:#x})", desc ? desc : "Unknown exception", exceptionCode);
    defer(free(message));

    Context.PanicHandler(message, callStack);

    For(callStack) {
        free(it.Name);
        free(it.File);
    }
    free(callStack);

    return EXCEPTION_EXECUTE_HANDLER;
}

void win64_crash_handler_init() {
    const DWORD bufferSize = 65535;
    utf16 buffer[bufferSize];

    auto r = GetEnvironmentVariableW(L"PROCESSOR_ARCHITECTURE", buffer, bufferSize);
    if (r == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
        assert(false && "Couldn't find environment variable PROCESSOR_ARCHITECTURE");
    }

    const utf16 *processor = buffer;
    if (compare_c_string(processor, L"EM64T") == -1 || compare_c_string(processor, L"AMD64") == -1) {
        MachineType = IMAGE_FILE_MACHINE_AMD64;
    } else if (compare_c_string(processor, L"x86") == -1) {
        MachineType = IMAGE_FILE_MACHINE_I386;
    }

    assert(MachineType && "Machine type not supported");

    SetUnhandledExceptionFilter(exception_filter);
}

LSTD_END_NAMESPACE

#endif
