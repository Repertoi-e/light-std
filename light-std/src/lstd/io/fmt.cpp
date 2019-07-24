#include "fmt.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

void parse_fmt_string(string_view fmtString, format_context *f) {
    parse_context *p = &f->Parse;
    p->Begin = p->It = fmtString.begin().to_pointer();
    p->End = fmtString.end().to_pointer();

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
                f->on_error("Unmatched '}' in format string - use double brackets (}}) to print a literal one");
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
            if (currentArg.Type == type::CUSTOM) {
                typename arg::handle(currentArg.Value.Custom).format(f);
            } else {
                visit_fmt_arg(format_context_visitor(f), currentArg);
            }
        } else if (*p->It == '{') {
            write(p->It + 1);
        } else if (*p->It == '!') {
            ++p->It;
            auto textStyle = p->parse_text_style();

            byte ansiBuffer[7 + 3 * 4 + 1];
            auto *ansiEnd = internal::color_to_ansii(ansiBuffer, textStyle);
            f->write_no_specs(ansiBuffer, ansiEnd - ansiBuffer);

            u8 emphasis = (u8) textStyle.Emphasis;
            if (emphasis) {
                assert(!textStyle.Background);
                ansiEnd = internal::emphasis_to_ansii(ansiBuffer, emphasis);
                f->write_no_specs(ansiBuffer, ansiEnd - ansiBuffer);
            }

            if (p->It == p->End || *p->It != '}') {
                f->on_error("Missing '}' in format string");
                return;
            }
        } else {
            currentArg = f->get_arg_from_ref(p->parse_arg_id());
            byte c = p->It != p->End ? *p->It : 0;
            if (c == '}') {
                if (currentArg.Type == type::CUSTOM) {
                    typename arg::handle(currentArg.Value.Custom).format(f);
                } else {
                    visit_fmt_arg(format_context_visitor(f), currentArg);
                }
            } else if (c == ':') {
                ++p->It;

                dynamic_format_specs specs = p->parse_fmt_specs(currentArg.Type);
                f->Specs = &specs;
                f->handle_dynamic_specs();
                defer(f->Specs = null);

                if (currentArg.Type == type::CUSTOM) {
                    typename arg::handle(currentArg.Value.Custom).format(f);
                } else {
                    visit_fmt_arg(format_context_visitor(f), currentArg);
                }
            } else {
                f->on_error("Missing '}' in format string");
                return;
            }
        }
        ++p->It;
    }
}
}  // namespace fmt

LSTD_END_NAMESPACE
