#include "fmt.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

void parse_fmt_string(string_view fmtString, format_context *f) {
    parse_context *p = &f->Parse;
    p->Begin = p->It = fmtString.begin();
    p->End = fmtString.end();

    auto write = [&](const byte *end) {
        if (p->It == end) return;
        while (true) {
            // @Cleanup Move this to a _find_ function
            auto *bracket = p->It;
            while (end - bracket > 4) {
                if (U32_HAS_VALUE(*(u32 *) bracket, '}')) break;
                bracket += 4;
            }

            while (bracket != end) {
                if (*bracket == '}') break;
                ++bracket;
            }

            if (bracket == end) {
                f->write_no_specs(p->It, end - p->It);
                return;
            }

            if (*(bracket + 1) != '}') {
                p->It = bracket;
                f->on_error("Unmatched '}' in format string - use '}}' to escape");
                return;
            }

            f->write_no_specs(p->It, bracket - p->It);
            f->write_no_specs("}");

            p->It = bracket + 2;
        }
    };

    arg currentArg;
    while (p->It != p->End) {
        // @Cleanup Move this to a _find_ function
        auto *bracket = p->It;
        while (p->End - bracket > 4) {
            if (U32_HAS_VALUE(*(u32 *) bracket, '{')) break;
            bracket += 4;
        }

        while (bracket != p->End) {
            if (*bracket == '{') break;
            ++bracket;
        }

        if (bracket == p->End) {
            write(p->End);
            return;
        }

        write(bracket);
        p->It = bracket + 1;

        if (p->It == p->End) {
            f->on_error("Invalid format string");
            return;
        }
        if (*p->It == '}') {
            currentArg = f->get_arg_from_ref(arg_ref(p->next_arg_id()));
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
            if (p->It == p->End || *p->It != '}') {
                f->on_error("'}' expected");
                return;
            }

            byte ansiiBuffer[7 + 3 * 4 + 1];
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

            byte c = p->It != p->End ? *p->It : 0;
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
                if (p->It == p->End || *p->It != '}') {
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
