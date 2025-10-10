#include "lstd/os.h"

#if OS == WINDOWS

#include "lstd/array.h"
#include "lstd/fmt.h"
#include "lstd/platform/windows/api.h"
#include "lstd/platform/windows/common.h"
#include "lstd/string.h"

LSTD_BEGIN_NAMESPACE

array<os_function_call> os_get_call_stack(s32 skipFrames, s32 maxDepth, void *platformContext) {
    array<os_function_call> callStack;

    if (maxDepth <= 0) return callStack;

    const s32 MAX_CAPTURE = 128;
    if (maxDepth > MAX_CAPTURE) maxDepth = MAX_CAPTURE;

    HANDLE process      = GetCurrentProcess();
    bool   symbolsReady = SymInitialize(process, null, true) != 0;
    if (symbolsReady) { SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME); }

    defer(if (symbolsReady) SymCleanup(process););

    auto push_frame = [&](DWORD64 address) {
        os_function_call call;
        call.LineNumber = -1;

        if (symbolsReady) {
            u8    symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            auto *symbol         = (PSYMBOL_INFO)symbolBuffer;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen   = MAX_SYM_NAME;

            DWORD64 displacement = 0;
            if (SymFromAddr(process, address, &displacement, symbol)) {
                call.Name = string(symbol->Name);
                if (!call.Name.Count) call.Name = "UnknownFunction";
            } else {
                call.Name = tprint("0x{:X}", address);
            }

            IMAGEHLP_LINEW64 lineInfo         = {sizeof(IMAGEHLP_LINEW64)};
            DWORD            lineDisplacement = 0;
            if (SymGetLineFromAddrW64(process, address, &lineDisplacement, &lineInfo)) {
                call.File = platform_utf16_to_utf8(lineInfo.FileName, platform_get_persistent_allocator());
                if (!call.File.Count) call.File = "UnknownFile";
                call.LineNumber = lineInfo.LineNumber;
            } else {
                call.File = "UnknownFile";
            }
        } else {
            call.Name = tprint("0x{:X}", address);
            call.File = "UnknownFile";
        }

        add(callStack, call);
    };

    if (platformContext) {
        auto *ctx = (CONTEXT *)platformContext;

        STACKFRAME64 frame;
        memset0((byte *)&frame, sizeof(STACKFRAME64));

        frame.AddrPC.Offset    = ctx->Rip;
        frame.AddrStack.Offset = ctx->Rsp;
        frame.AddrFrame.Offset = ctx->Rbp;
        frame.AddrPC.Mode      = AddrModeFlat;
        frame.AddrStack.Mode   = AddrModeFlat;
        frame.AddrFrame.Mode   = AddrModeFlat;

        s32 skipped = skipFrames;
        while ((s32)callStack.Count < maxDepth &&
               StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, GetCurrentThread(), &frame, ctx, null, SymFunctionTableAccess64, SymGetModuleBase64, null)) {
            if (frame.AddrFrame.Offset == 0) break;
            if (skipped > 0) {
                --skipped;
                continue;
            }
            push_frame(frame.AddrPC.Offset);
        }
    } else {
        void *frames[MAX_CAPTURE];
        ULONG captured = RtlCaptureStackBackTrace((ULONG)skipFrames + 1, (ULONG)maxDepth, frames, null);
        for (ULONG i = 0; i < captured && (s32)callStack.Count < maxDepth; ++i) { push_frame((DWORD64)frames[i]); }
    }

    return callStack;
}

LSTD_END_NAMESPACE

#endif
