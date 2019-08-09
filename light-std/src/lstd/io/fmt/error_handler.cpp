#include "error_handler.h"

#include "../../os.h"
#include "../fmt.h"

LSTD_BEGIN_NAMESPACE
namespace fmt {
void default_error_handler(string message, error_context errorContext) {
    // An error during formatting occured.
    // If you are running a debugger it has now hit a breakpoint.

    // Make escape characters appear as they would in a string literal
    errorContext.FmtString.replace_all('\"', "\\\"")
        ->replace_all('\\', "\\\\")
        ->replace_all('\a', "\\a")
        ->replace_all('\b', "\\b")
        ->replace_all('\f', "\\f")
        ->replace_all('\n', "\\n")
        ->replace_all('\r', "\\r")
        ->replace_all('\t', "\\t")
        ->replace_all('\v', "\\v");

    fmt::print("\n\n {!GRAY} An error during formatting occured: {!YELLOW}{}{!GRAY}\n", message);
    fmt::print("    ... the error happened here:\n");
    fmt::print("        {!}{}{!GRAY}\n", errorContext.FmtString);
    fmt::print("        {: >{}} {!} \n\n", "^", errorContext.Position + 1);
#if defined NDEBUG
    os_exit();
#else
    // More info has been printed to the console but here's the error message:
    auto errorMessage = message;
    assert(false);
#endif
}
}  // namespace fmt

LSTD_END_NAMESPACE
