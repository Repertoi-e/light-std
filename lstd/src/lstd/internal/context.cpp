#include "context.h"

#include "../memory/array.h"

LSTD_BEGIN_NAMESPACE

void default_panic_handler(const string &message, const array<os_function_call> &callStack) {
    fmt::print("\n\n{!}(context.cpp / default_crash_handler): An panic occured and the program must terminate.\n");
    fmt::print("{!GRAY}        Error: {!RED}{}{!}\n\n", message);
    fmt::print("        ... and here is the call stack:\n");
    if (callStack.Count) {
        fmt::print("\n");
    }
    For(callStack) {
        fmt::print("        {!YELLOW}{}{!}\n", it.Name);
        fmt::print("          in file: {}:{}\n", it.File, it.LineNumber);
    }
    if (!callStack.Count) {
        fmt::print("          [No call stack available]\n");
    }
    fmt::print("\n\n");
}

LSTD_END_NAMESPACE
