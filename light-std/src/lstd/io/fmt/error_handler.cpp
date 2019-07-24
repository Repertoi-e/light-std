#include "error_handler.h"

#include "../fmt.h"

LSTD_BEGIN_NAMESPACE
namespace fmt {
void default_error_handler(const byte *message, error_context errorContext) {
    // An error during formatting occured.
    // If you are running a debugger it has now hit a breakpoint.

    // Make escape characters appear as they would in a string literal
    string entireString = errorContext.FmtString;
    size_t index = 0;
    while (index < entireString.Length) {
        auto it = entireString[index];

        // @Cleanup Add code point methods in string's api
        // @Speed Implement faster string searching (using U32_HAS_VALUE)
#define HANDLE_ESCAPED(a, b)                                        \
    if (it == a) {                                                  \
        entireString.remove(index);                                 \
        entireString.insert(index, b);                              \
        ++index;                                                    \
        if (index < errorContext.Position) ++errorContext.Position; \
    }
        HANDLE_ESCAPED('\"', "\\\"");
        HANDLE_ESCAPED('\"', "\\\"");
        HANDLE_ESCAPED('\\', "\\\\");
        HANDLE_ESCAPED('\a', "\\a");
        HANDLE_ESCAPED('\b', "\\b");
        HANDLE_ESCAPED('\f', "\\f");
        HANDLE_ESCAPED('\n', "\\n");
        HANDLE_ESCAPED('\r', "\\r");
        HANDLE_ESCAPED('\t', "\\t");
        HANDLE_ESCAPED('\v', "\\v");

        ++index;
    }

    fmt::print("\n\n {!GRAY} An error during formatting occured: {!YELLOW}{}{!GRAY}\n", message);
    fmt::print("    ... the error happened here:\n");
    fmt::print("        {!}{}{!GRAY}\n", entireString);
    fmt::print("        {: >{}} {!} \n\n", "^", errorContext.Position + 1);
#if defined NDEBUG
    // @TODO: Exit the program
#else
    // More info has been printed to the console but here's the error message:
    auto errorMessage = message;
    assert(false);
#endif
}
}  // namespace fmt

LSTD_END_NAMESPACE
