#include "context.h"

#include "io/fmt.h"
#include "storage/array.h"

void default_unexpected_exception_handler(string message, array<os_function_call> callStack) {
    fmt::print("------------------\n");
    fmt::print("An unexpected exception occured and the program must terminate.\n");
    fmt::print("Here is the code: {}\n", message);
    fmt::print("...and here is the call stack:\n");
    For(callStack) {
        fmt::print("    {}\n", it.Name);
        fmt::print("    in file: {}:{}\n", it.File, it.LineNumber);
    }
    fmt::print("------------------\n");
}
