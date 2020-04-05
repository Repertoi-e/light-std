#include "context.h"

#include "io/fmt.h"
#include "storage/array.h"

void default_unexpected_exception_handler(string message, array<os_function_call> callStack) {
    fmt::print("\n{!}(context.cpp / default_crash_handler): An exception occured and the program must terminate.\n");
    fmt::print("{!GRAY}        Error: {!RED}{}{!}\n\n", message);
    fmt::print("        ... and here is the call stack:\n");
    For(callStack) {
        fmt::print("        {!YELLOW}{}{!}\n", it.Name);
        fmt::print("          in file: {}:{}\n", it.File, it.LineNumber);
    }
    fmt::print("\n\n");
}
