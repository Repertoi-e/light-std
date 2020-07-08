#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/io.h"
#include "lstd/io/fmt.h"
#include "lstd/memory/table.h"
#include "lstd/os.h"

LSTD_BEGIN_NAMESPACE

#define CALLSTACK_DEPTH 6

static DWORD MachineType;
static table<DWORD, const char *> CodeDescs;

static LONG exception_filter(LPEXCEPTION_POINTERS e) {
    u32 exceptionCode = e->ExceptionRecord->ExceptionCode;

    HANDLE hProcess = INVALID_HANDLE_VALUE;
    defer(SymCleanup(hProcess));

    if (!SymInitialize(GetCurrentProcess(), null, true)) return EXCEPTION_EXECUTE_HANDLER;

    auto c = e->ContextRecord;

    STACKFRAME64 sf;
    memset(&sf, 0, sizeof(STACKFRAME));
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
            if (call.Name.Length == 0) call.Name = "UnknownFunction";  // @Leak
        }

        IMAGEHLP_LINE64 lineInfo = {sizeof(IMAGEHLP_LINE64)};

        DWORD lineDisplacement = 0;
        if (SymGetLineFromAddr64(hProcess, sf.AddrPC.Offset, &lineDisplacement, &lineInfo)) {
            clone(&call.File, string(lineInfo.FileName));
            if (call.File.Length == 0) call.File = "UnknownFile";  // @Leak

            call.LineNumber = lineInfo.LineNumber;
        }

        move(callStack.append(), &call);
    }

    auto *desc = CodeDescs.find(exceptionCode);
    string message;
    fmt::sprint(&message, "{} ({:#x})", desc ? *desc : "Unknown exception", exceptionCode);
    Context.UnexpectedExceptionHandler(message, callStack);

    return EXCEPTION_EXECUTE_HANDLER;
}

void win32_crash_handler_init() {
    wchar_t *processor = _wgetenv(L"PROCESSOR_ARCHITECTURE");  // @NoCRT
    if (processor) {
        // @NoCRT (wcscmp)
        if ((!wcscmp(L"EM64T", processor)) || !wcscmp(L"AMD64", processor)) {
            MachineType = IMAGE_FILE_MACHINE_AMD64;
        } else if (!wcscmp(L"x86", processor)) {
            MachineType = IMAGE_FILE_MACHINE_I386;
        }
    }
    assert(MachineType && "Machine type not supported");

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

    run_at_exit([]() { CodeDescs.release(); });

    SetUnhandledExceptionFilter(exception_filter);
}

LSTD_END_NAMESPACE

#endif