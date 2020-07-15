#include "fmt.h"

#include "../file.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

void parse_fmt_string(const string &fmtString, format_context *f) {
    parse_context *p = &f->Parse;

    auto write = [&](const char *end) {
        if (p->It == end) return;
        while (true) {
            auto *bracket = find_cp_utf8(p->It, end - p->It, '}');
            if (!bracket) {
                f->write_no_specs(p->It, end - p->It);
                return;
            }

            if (*(bracket + 1) != '}') {
                p->It = bracket;  // To help error reporting at the right place.
                f->on_error("Unmatched '}' in format string - use '}}' to escape");
                return;
            }

            f->write_no_specs(p->It, bracket - p->It);
            f->write_no_specs("}");

            p->It = bracket + 2;
        }
    };

    arg currentArg;

    auto end = fmtString.Data + fmtString.ByteLength;
    while (p->It != end) {
        auto *bracket = find_cp_utf8(p->It, end - p->It, '{');
        if (!bracket) {
            write(end);
            return;
        }

        write(bracket);
        p->It = bracket + 1;

        if (p->It == end) {
            f->on_error("Invalid format string");
            return;
        }
        if (*p->It == '}') {
            auto argId = p->next_arg_id();
            currentArg = f->get_arg_from_ref(arg_ref(argId));
            if (currentArg.Type == type::NONE) return;  // The error was reported in _f->get_arg_from_ref_

            if (currentArg.Type == type::CUSTOM) {
                typename arg::handle(currentArg.Value.Custom).format(f);
            } else {
                visit_fmt_arg(format_context_visitor(f), currentArg);
            }
        } else if (*p->It == '{') {
            write(p->It + 1);
        } else if (*p->It == '!') {
            ++p->It;
            text_style style = {};
            bool success = p->parse_text_style(&style);
            if (!success) return;
            if (p->It == end || *p->It != '}') {
                f->on_error("'}' expected");
                return;
            }

            char ansiiBuffer[7 + 3 * 4 + 1];
            auto *ansiiEnd = internal::color_to_ansii(ansiiBuffer, style);
            f->write_no_specs(ansiiBuffer, ansiiEnd - ansiiBuffer);

            u8 emphasis = (u8) style.Emphasis;
            if (emphasis) {
                assert(!style.Background);
                ansiiEnd = internal::emphasis_to_ansii(ansiiBuffer, emphasis);
                f->write_no_specs(ansiiBuffer, ansiiEnd - ansiiBuffer);
            }
        } else {
            currentArg = f->get_arg_from_ref(p->parse_arg_id());
            if (currentArg.Type == type::NONE) return;  // The error was reported in _f->get_arg_from_ref_

            char c = p->It != end ? *p->It : 0;
            if (c == '}') {
                if (currentArg.Type == type::CUSTOM) {
                    typename arg::handle(currentArg.Value.Custom).format(f);
                } else {
                    visit_fmt_arg(format_context_visitor(f), currentArg);
                }
            } else if (c == ':') {
                ++p->It;

                dynamic_format_specs specs = {};
                bool success = p->parse_fmt_specs(currentArg.Type, &specs);
                if (!success) return;
                if (p->It == end || *p->It != '}') {
                    f->on_error("'}' expected");
                    return;
                }

                f->Specs = &specs;
                success = f->handle_dynamic_specs();
                if (!success) return;

                defer(f->Specs = null);

                if (currentArg.Type == type::CUSTOM) {
                    typename arg::handle(currentArg.Value.Custom).format(f);
                } else {
                    visit_fmt_arg(format_context_visitor(f), currentArg);
                }
            } else {
                f->on_error("'}' expected");
                return;
            }
        }
        ++p->It;
    }
}
}  // namespace fmt

LSTD_END_NAMESPACE
