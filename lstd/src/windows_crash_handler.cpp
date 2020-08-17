#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/io.h"
#include "lstd/memory/table.h"
#include "lstd/os.h"

LSTD_BEGIN_NAMESPACE

#define CALLSTACK_DEPTH 6

file_scope DWORD MachineType;
file_scope hash_table<DWORD, const char *> CodeDescs;

file_scope LONG exception_filter(LPEXCEPTION_POINTERS e) {
    u32 exceptionCode = e->ExceptionRecord->ExceptionCode;

    HANDLE hProcess = INVALID_HANDLE_VALUE;
    defer(SymCleanup(hProcess));

    if (!SymInitialize(GetCurrentProcess(), null, true)) return EXCEPTION_EXECUTE_HANDLER;

    auto c = e->ContextRecord;

    STACKFRAME64 sf;
    fill_memory(&sf, 0, sizeof(STACKFRAME));

    sf.AddrPC.Offset = c->Rip;
    sf.AddrStack.Offset = c->Rsp;
    sf.AddrFrame.Offset = c->Rbp;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrStack.Mode = AddrModeFlat;
    sf.AddrFrame.Mode = AddrModeFlat;

    array<os_function_call> callStack;

    while (StackWalk64(MachineType, GetCurrentProcess(), GetCurrentThread(), &sf, c, 0, SymFunctionTableAccess64,
                       SymGetModuleBase64, null)) {
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
                call.Name.release();
                call.Name = "UnknownFunction";
            }
        }

        IMAGEHLP_LINE64 lineInfo = {sizeof(IMAGEHLP_LINE64)};

        DWORD lineDisplacement = 0;
        if (SymGetLineFromAddr64(hProcess, sf.AddrPC.Offset, &lineDisplacement, &lineInfo)) {
            clone(&call.File, string(lineInfo.FileName));
            if (call.File.Length == 0) {
                call.File.release();
                call.File = "UnknownFile";
            }
            call.LineNumber = lineInfo.LineNumber;
        }

        callStack.append(call);
    }

    auto desc = CodeDescs.find(exceptionCode).second;

    string message = fmt::sprint("{} ({:#x})", desc ? *desc : "Unknown exception", exceptionCode);
    defer(message.release());

    Context.PanicHandler(message, callStack);

    For(callStack) {
        it.Name.release();
        it.File.release();
    }
    callStack.release();

    return EXCEPTION_EXECUTE_HANDLER;
}

void release_code_descs() {
    CodeDescs.release();
}

void win32_crash_handler_init() {
    auto [success, processor] = os_get_env("PROCESSOR_ARCHITECTURE");
    defer(processor.release());

    if (success) {
        if (processor == "EM64T" || processor == "AMD64") {
            MachineType = IMAGE_FILE_MACHINE_AMD64;
        } else if (processor == "x86") {
            MachineType = IMAGE_FILE_MACHINE_I386;
        }
    }
    assert(MachineType && "Machine type not supported");

    exit_schedule(release_code_descs);

#define CODE_DESCR(code) code, #code
    CodeDescs.add(CODE_DESCR(EXCEPTION_ACCESS_VIOLATION));
    CodeDescs.add(CODE_DESCR(EXCEPTION_DATATYPE_MISALIGNMENT));
    CodeDescs.add(CODE_DESCR(EXCEPTION_BREAKPOINT));
    CodeDescs.add(CODE_DESCR(EXCEPTION_SINGLE_STEP));
    CodeDescs.add(CODE_DESCR(EXCEPTION_ARRAY_BOUNDS_EXCEEDED));
    CodeDescs.add(CODE_DESCR(EXCEPTION_FLT_DENORMAL_OPERAND));
    CodeDescs.add(CODE_DESCR(EXCEPTION_FLT_DIVIDE_BY_ZERO));
    CodeDescs.add(CODE_DESCR(EXCEPTION_FLT_INEXACT_RESULT));
    CodeDescs.add(CODE_DESCR(EXCEPTION_FLT_INVALID_OPERATION));
    CodeDescs.add(CODE_DESCR(EXCEPTION_FLT_OVERFLOW));
    CodeDescs.add(CODE_DESCR(EXCEPTION_FLT_STACK_CHECK));
    CodeDescs.add(CODE_DESCR(EXCEPTION_FLT_UNDERFLOW));
    CodeDescs.add(CODE_DESCR(EXCEPTION_INT_DIVIDE_BY_ZERO));
    CodeDescs.add(CODE_DESCR(EXCEPTION_INT_OVERFLOW));
    CodeDescs.add(CODE_DESCR(EXCEPTION_PRIV_INSTRUCTION));
    CodeDescs.add(CODE_DESCR(EXCEPTION_IN_PAGE_ERROR));
    CodeDescs.add(CODE_DESCR(EXCEPTION_ILLEGAL_INSTRUCTION));
    CodeDescs.add(CODE_DESCR(EXCEPTION_NONCONTINUABLE_EXCEPTION));
    CodeDescs.add(CODE_DESCR(EXCEPTION_STACK_OVERFLOW));
    CodeDescs.add(CODE_DESCR(EXCEPTION_INVALID_DISPOSITION));
    CodeDescs.add(CODE_DESCR(EXCEPTION_GUARD_PAGE));
    CodeDescs.add(CODE_DESCR(EXCEPTION_INVALID_HANDLE));
    // CodeDescs.add(CODE_DESCR(EXCEPTION_POSSIBLE_DEADLOCK));

    SetUnhandledExceptionFilter(exception_filter);
}

LSTD_END_NAMESPACE

#endif