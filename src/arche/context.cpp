#include "arche/context.h"

#include "arche/fmt.h"
#include "arche/os.h"

ARCHE_BEGIN_NAMESPACE

void fmt_default_parse_error_handler(string message, string formatString, s64 position) {
    // An error during formatting occured.
    // If you are running a debugger it has now hit a breakpoint.
    //
    // You can replace this error handler in the Context with a less critical one.

    string str = clone(formatString);
    defer(free(str));

    auto p = [](auto cp) {
        return has(string("\"\\\a\b\f\n\r\t\v"), cp);
    };

    s64 start                                                         = 0;
    s64 countExtraSlashesToBeAddedBeforePositionToAccountWhenPrinting = 0;
    while (start <= position && (start = search(str, &p, .Start = start + 1)) != -1) { countExtraSlashesToBeAddedBeforePositionToAccountWhenPrinting += 1; }

    // Make escape characters appear as they would in a string literal
    replace_all(str, '\"', "\\\"");
    replace_all(str, '\\', "\\\\");
    replace_all(str, '\a', "\\a");
    replace_all(str, '\b', "\\b");
    replace_all(str, '\f', "\\f");
    replace_all(str, '\n', "\\n");
    replace_all(str, '\r', "\\r");
    replace_all(str, '\t', "\\t");
    replace_all(str, '\v', "\\v");

    string_builder b;
    defer(free(b));

    string_builder_writer output;
    output.Builder = &b;

    fmt_to_writer(&output, "\n\n>>> {!GRAY}An error during formatting occured: {!YELLOW}{}{!GRAY}\n", message);
    fmt_to_writer(&output, "    ... the error happened here:\n");
    fmt_to_writer(&output, "        {!}{}{!GRAY}\n", str);

    s64 spaces = position + countExtraSlashesToBeAddedBeforePositionToAccountWhenPrinting + 1;
    fmt_to_writer(&output, "        {: >{}} {!} \n\n", "^", spaces);

    string info = builder_to_string(b);
    defer(free(info));

    print("{}", info);

#if defined NDEBUG
    panic("Error in fmt.h");
#else
    // More info has been printed to the console but here's the error message:
    auto errorMessage = message;
    assert(false);
#endif
}

ARCHE_END_NAMESPACE

#if defined ARCHE_THROW_ON_ASSERT_THROW_ON_PANIC
#undef search
#undef ref

#include <string>
#include <stdexcept>
#endif

ARCHE_BEGIN_NAMESPACE

//
// This file includes the base implementations of panic handlers,
// see :Context: in context.h to see how to override them.
//

void default_panic_handler(string message, array<os_function_call> callStack) {
    if (Context._HandlingPanic) return;

    auto newContext           = Context;
    newContext._HandlingPanic = true;

#if defined ARCHE_THROW_ON_ASSERT_THROW_ON_PANIC
    newContext.FmtDisableAnsiCodes = true;
#endif

    string_builder b;

    string_builder_writer writer;
    writer.Builder = &b;

    PUSH_CONTEXT(newContext) {
        fmt_to_writer(&writer,
            "\n\n{!}(context.cpp / default_crash_handler): A panic occurred and "
            "the program must terminate.\n");
        fmt_to_writer(&writer, "{!GRAY}        Error: {!RED}{}{!}\n\n", message);
        fmt_to_writer(&writer, "        ... and here is the call stack:\n");
        if (callStack.Count) { fmt_to_writer(&writer, "\n"); }
        For(callStack) {
            fmt_to_writer(&writer, "        {!YELLOW}{}{!}\n", it.Name);
            fmt_to_writer(&writer, "          in file: {}:{}\n", it.File, it.LineNumber);
        }
        if (!callStack.Count) { fmt_to_writer(&writer, "          [No call stack available]\n"); }
        fmt_to_writer(&writer, "\n\n");
    }

    string info = builder_to_string(b);

#if defined ARCHE_THROW_ON_ASSERT_THROW_ON_PANIC
    throw std::logic_error(std::string(to_c_string(info)));
#else
    print("{}", info);
#if DEBUG
    debug_break();
#else
    exit(-1);
#endif
#endif
}

ARCHE_END_NAMESPACE
