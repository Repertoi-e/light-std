#include "fmt.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

void parse_fmt_string(const string &fmtString, format_context *f) {
    parse_context *p = &f->Parse;

    auto write_until = [&](const char *end) {
        if (!p->It.Count) return;
        while (true) {
            auto *bracket = find_cp_utf8(p->It.Data, end - p->It.Data, '}');
            if (!bracket) {
                f->write_no_specs(p->It.Data, end - p->It.Data);
                return;
            }

            if (*(bracket + 1) != '}') {
                f->on_error("Unmatched \"}\" in format string - if you want to print it use \"}}\" to escape", bracket - f->Parse.FormatString.Data);
                return;
            }

            f->write_no_specs(p->It.Data, bracket - p->It.Data);
            f->write_no_specs("}");

            s64 advance = bracket + 2 - p->It.Data;
            p->It.Data += advance, p->It.Count -= advance;
        }
    };

    arg currentArg;

    while (p->It.Count) {
        auto *bracket = find_cp_utf8(p->It.Data, p->It.Count, '{');
        if (!bracket) {
            write_until(p->It.Data + p->It.Count);
            return;
        }

        write_until(bracket);

        s64 advance = bracket + 1 - p->It.Data;
        p->It.Data += advance, p->It.Count -= advance;

        if (!p->It.Count) {
            f->on_error("Invalid format string");
            return;
        }
        if (p->It[0] == '}') {
            // Implicit {} means "get the next argument"
            currentArg = f->get_arg_from_index(p->next_arg_id());
            if (currentArg.Type == type::NONE) return;  // The error was reported in _f->get_arg_from_ref_

            visit_fmt_arg(internal::format_context_visitor(f), currentArg);
        } else if (p->It[0] == '{') {
            // {{ means we escaped a {.
            write_until(p->It.Data + 1);
        } else if (p->It[0] == '!') {
            ++p->It.Data, --p->It.Count;  // Skip the !

            text_style style = {};
            bool success = p->parse_text_style(&style);
            if (!success) return;
            if (!p->It.Count || p->It[0] != '}') {
                f->on_error("\"}\" expected");
                return;
            }

            if (!Context.FmtDisableAnsiCodes) {
                char ansiBuffer[7 + 3 * 4 + 1];
                auto *ansiEnd = internal::color_to_ansi(ansiBuffer, style);
                f->write_no_specs(ansiBuffer, ansiEnd - ansiBuffer);

                u8 emphasis = (u8) style.Emphasis;
                if (emphasis) {
                    assert(!style.Background);
                    ansiEnd = internal::emphasis_to_ansi(ansiBuffer, emphasis);
                    f->write_no_specs(ansiBuffer, ansiEnd - ansiBuffer);
                }
            }
        } else {
            // Parse integer specified or a named argument
            s64 argId = p->parse_arg_id();
            if (argId == -1) return;

            currentArg = f->get_arg_from_index(argId);
            if (currentArg.Type == type::NONE) return;  // The error was reported in _f->get_arg_from_ref_

            char c = p->It.Count ? p->It[0] : 0;
            if (c == '}') {
                visit_fmt_arg(internal::format_context_visitor(f), currentArg);
            } else if (c == ':') {
                ++p->It.Data, --p->It.Count;  // Skip the :

                dynamic_format_specs specs = {};
                bool success = p->parse_fmt_specs(currentArg.Type, &specs);
                if (!success) return;
                if (!p->It.Count || p->It[0] != '}') {
                    f->on_error("\"}\" expected");
                    return;
                }

                f->Specs = &specs;
                success = f->handle_dynamic_specs();
                if (!success) return;

                visit_fmt_arg(internal::format_context_visitor(f), currentArg);

                f->Specs = null;
            } else {
                f->on_error("\"}\" expected");
                return;
            }
        }
        ++p->It.Data, --p->It.Count;  // Go to the next byte
    }
}
}  // namespace fmt

LSTD_END_NAMESPACE
