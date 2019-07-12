#pragma once

#include "../../common.h"

#include "../../storage/string_utils.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct error_context {
    string_view FmtString;
    size_t Position;
};

using error_handler_t = void (*)(const byte *message, error_context errorContext);

// This function is intentionally not constexpr to give a compile-time error (if we run compile-time)
inline void default_error_handler(const byte *message, error_context errorContext) {
    // An error during formatting occured.
    // If you are running a debugger it has now hit a breakpoint.

    // Here's the error message:
    auto errorMessage = message;

    // .. and here is the context:
    auto entireString = errorContext.FmtString;
    auto fromPositionOnwards = entireString.substring(errorContext.Position, entireString.Length);

#if 0 && defined NDEBUG
    // If running in release, then print the error in the console.
	// @TODO: cout is not a member of io::
    io::cout.write("\nAn error during formatting occured: ");
    io::cout.write(message);
    io::cout.write("\n");
    io::cout.write("    ... the error happened here:\n");
    io::cout.write("        ");
    io::cout.write(entireString);
    io::cout.write("\n");
    io::cout.write("        ");
    For(range(errorContext.Position)) io::cout.write(' ');
    io::cout.write("^\n\n");

    io::cout.flush();
    // @TODO: Exit the program
#else
    assert(false);
#endif
}
}  // namespace fmt

LSTD_END_NAMESPACE
