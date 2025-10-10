#include "lstd/os.h"

#if (OS == LINUX) || (OS == MACOS)

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "lstd/array.h"
#include "lstd/fmt.h"
#include "lstd/string.h"

LSTD_BEGIN_NAMESPACE

array<os_function_call> os_get_call_stack(s32 skipFrames, s32 maxDepth, void *platformContext) {
    array<os_function_call> callStack;
    if (maxDepth <= 0) return callStack;

    const s32 MAX_CAPTURE = 128;
    if (maxDepth > MAX_CAPTURE) maxDepth = MAX_CAPTURE;

    // Capture a few extra frames so we can skip the requested amount plus this
    // helper.
    void *frames[MAX_CAPTURE + 32];
    (void)platformContext;

    s32 frameCapacity = maxDepth + skipFrames + 1;
    if (frameCapacity > (s32)(sizeof(frames) / sizeof(frames[0]))) { frameCapacity = sizeof(frames) / sizeof(frames[0]); }

    s32 captured = backtrace(frames, frameCapacity);
    if (captured <= 0) return callStack;

    s32 start = skipFrames + 1;
    if (start >= captured) return callStack;

    auto resolve_source_location = [](const Dl_info &info, void *address, string &fileOut) -> s32 {
        if (!info.dli_fname) return -1;

        char command[1024];
#if OS == MACOS
        snprintf(command, sizeof(command), "atos -o \"%s\" -l %p %p", info.dli_fname, info.dli_fbase, address);
#else
        uintptr_t rel_addr = (uintptr_t)address - (uintptr_t)info.dli_fbase;
        snprintf(command, sizeof(command), "addr2line -Cfpe \"%s\" %p", info.dli_fname, rel_addr);
#endif

        FILE *pipe = popen(command, "r");
        if (!pipe) return -1;

        char line[1024];
        s32  lineNumber = -1;

#if OS == MACOS
        if (fgets(line, sizeof(line), pipe)) {
            char *open  = strrchr(line, '(');
            char *close = strrchr(line, ')');
            char *colon = open ? strrchr(open, ':') : null;
            if (colon && close && colon < close) {
                *close     = '\0';
                lineNumber = (s32)atoi(colon + 1);
                *colon     = '\0';
                char *path = open + 1;
                while (*path == ' ') ++path;
                char *newline = strchr(path, '\n');
                if (newline) *newline = '\0';
                if (*path) fileOut = make_string(path);
            }
        }
#else
        // Skip demangled function line if present
        if (fgets(line, sizeof(line), pipe)) {
            // ignore function name output
        }
        if (fgets(line, sizeof(line), pipe)) {
            char *newline = strchr(line, '\n');
            if (newline) *newline = '\0';
            char *colon = strrchr(line, ':');
            if (colon) {
                lineNumber = (s32)atoi(colon + 1);
                *colon     = '\0';
                if (*line) fileOut = make_string(line);
            }
        }
#endif

        pclose(pipe);
        return lineNumber;
    };

    for (s32 i = start; i < captured && callStack.Count < maxDepth; ++i) {
        Dl_info info;
        memset0(&info, sizeof(info));

        os_function_call call;
        call.LineNumber = -1;

        if (dladdr(frames[i], &info) && info.dli_sname) {
            int   status    = 0;
            char *demangled = abi::__cxa_demangle(info.dli_sname, null, null, &status);
            if (status == 0 && demangled) {
                call.Name = make_string(demangled);
            } else {
                call.Name = make_string(info.dli_sname);
            }
        } else {
            call.Name = tprint("0x{:X}", (u64)frames[i]);
        }

        s32 lineNumber = resolve_source_location(info, frames[i], call.File);
        if (lineNumber != -1) call.LineNumber = lineNumber;

        add(callStack, call);
    }

    return callStack;
}

LSTD_END_NAMESPACE

#endif
