#include "lstd/internal/common.h"

#if OS == WINDOWS
#include "lstd/internal/os_function_call.h"
#include "lstd/io.h"
#include "lstd/memory/hash_table.h"
#include "lstd/os.h"
#include "lstd/types/windows.h"

import fmt;

LSTD_BEGIN_NAMESPACE

#define CALLSTACK_DEPTH 6

file_scope DWORD MachineType;
file_scope hash_table<DWORD, const char *> CodeDescs;

// @TODO: Factor the stack walking part of this function into a os_get_call_stack() which can be used anywhere in the program.

file_scope LONG exception_filter(LPEXCEPTION_POINTERS e) {
    u32 exceptionCode = e->ExceptionRecord->ExceptionCode;

    HANDLE hProcess = INVALID_HANDLE_VALUE;
    defer(SymCleanup(hProcess));

    if (!SymInitialize(GetCurrentProcess(), null, true)) return EXCEPTION_EXECUTE_HANDLER;

    auto c = e->ContextRecord;

    STACKFRAME64 sf;
    fill_memory(&sf, 0, sizeof(STACKFRAME64));

    sf.AddrPC.Offset = c->Rip;
    sf.AddrStack.Offset = c->Rsp;
    sf.AddrFrame.Offset = c->Rbp;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrStack.Mode = AddrModeFlat;
    sf.AddrFrame.Mode = AddrModeFlat;

    array<os_function_call> callStack;

    while (StackWalk64(MachineType, GetCurrentProcess(), GetCurrentThread(), &sf, c, 0, SymFunctionTableAccess64, SymGetModuleBase64, null)) {
        if (sf.AddrFrame.Offset == 0 || callStack.Count >= CALLSTACK_DEPTH) break;

        constexpr auto s = (sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64);
        ULONG64 symbolBuffer[s];

        PSYMBOL_INFO symbol = (PSYMBOL_INFO) symbolBuffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        os_function_call call;

        DWORD64 symDisplacement = 0;
        if (SymFromAddr(hProcess, sf.AddrPC.Offset, &symDisplacement, symbol)) {
            clone(&call.Name, string(symbol->Name));
            if (call.Name.Length == 0) {
                free(call.Name);
                call.Name = "UnknownFunction";
            }
        }

        IMAGEHLP_LINEW64 lineInfo = {sizeof(IMAGEHLP_LINEW64)};

        DWORD lineDisplacement = 0;
        if (SymGetLineFromAddrW64(hProcess, sf.AddrPC.Offset, &lineDisplacement, &lineInfo)) {
            clone(&call.File, string(lineInfo.FileName));
            if (call.File.Length == 0) {
                free(call.File);
                call.File = "UnknownFile";
            }
            call.LineNumber = lineInfo.LineNumber;
        }

        append(callStack, call);
    }

    auto desc = find(CodeDescs, exceptionCode).Value;

    string message = sprint("{} ({:#x})", desc ? *desc : "Unknown exception", exceptionCode);
    defer(free(message));

    Context.PanicHandler(message, callStack);

    For(callStack) {
        free(it.Name);
        free(it.File);
    }
    free(callStack);

    return EXCEPTION_EXECUTE_HANDLER;
}

void release_code_descs() {
    free(CodeDescs);
}

void win32_crash_handler_init() {
    auto [processor, success] = os_get_env("PROCESSOR_ARCHITECTURE");
    if (success) {
        defer(free(processor));
        if (processor == "EM64T" || processor == "AMD64") {
            MachineType = IMAGE_FILE_MACHINE_AMD64;
        } else if (processor == "x86") {
            MachineType = IMAGE_FILE_MACHINE_I386;
        }
    }
    assert(MachineType && "Machine type not supported");

    exit_schedule(release_code_descs);

#define CODE_DESCR(code) code, #code
    add(CodeDescs, CODE_DESCR(EXCEPTION_ACCESS_VIOLATION));
    add(CodeDescs, CODE_DESCR(EXCEPTION_DATATYPE_MISALIGNMENT));
    add(CodeDescs, CODE_DESCR(EXCEPTION_BREAKPOINT));
    add(CodeDescs, CODE_DESCR(EXCEPTION_SINGLE_STEP));
    add(CodeDescs, CODE_DESCR(EXCEPTION_ARRAY_BOUNDS_EXCEEDED));
    add(CodeDescs, CODE_DESCR(EXCEPTION_FLT_DENORMAL_OPERAND));
    add(CodeDescs, CODE_DESCR(EXCEPTION_FLT_DIVIDE_BY_ZERO));
    add(CodeDescs, CODE_DESCR(EXCEPTION_FLT_INEXACT_RESULT));
    add(CodeDescs, CODE_DESCR(EXCEPTION_FLT_INVALID_OPERATION));
    add(CodeDescs, CODE_DESCR(EXCEPTION_FLT_OVERFLOW));
    add(CodeDescs, CODE_DESCR(EXCEPTION_FLT_STACK_CHECK));
    add(CodeDescs, CODE_DESCR(EXCEPTION_FLT_UNDERFLOW));
    add(CodeDescs, CODE_DESCR(EXCEPTION_INT_DIVIDE_BY_ZERO));
    add(CodeDescs, CODE_DESCR(EXCEPTION_INT_OVERFLOW));
    add(CodeDescs, CODE_DESCR(EXCEPTION_PRIV_INSTRUCTION));
    add(CodeDescs, CODE_DESCR(EXCEPTION_IN_PAGE_ERROR));
    add(CodeDescs, CODE_DESCR(EXCEPTION_ILLEGAL_INSTRUCTION));
    add(CodeDescs, CODE_DESCR(EXCEPTION_NONCONTINUABLE_EXCEPTION));
    add(CodeDescs, CODE_DESCR(EXCEPTION_STACK_OVERFLOW));
    add(CodeDescs, CODE_DESCR(EXCEPTION_INVALID_DISPOSITION));
    add(CodeDescs, CODE_DESCR(EXCEPTION_GUARD_PAGE));
    add(CodeDescs, CODE_DESCR(EXCEPTION_INVALID_HANDLE));
    add(CodeDescs, CODE_DESCR(EXCEPTION_POSSIBLE_DEADLOCK));

    SetUnhandledExceptionFilter(exception_filter);
}

LSTD_END_NAMESPACE

#endif