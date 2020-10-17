#include "fmt.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

type get_type(args ars, s64 index) {
    u64 shift = (u64) index * 4;
    return (type)((ars.Types & (0xfull << shift)) >> shift);
}

// Doesn't support negative indexing
arg get_arg(args ars, s64 index) {
    if (index >= ars.Count) return {};

    if (!(ars.Types & IS_UNPACKED_BIT)) {
        if (index > MAX_PACKED_ARGS) return {};

        auto type = get_type(ars, index);
        if (type == type::NONE) return {};

        arg result;
        result.Type = type;
        result.Value = ((value *) ars.Data)[index];
        return result;
    }
    return ((arg *) ars.Data)[index];
}

// Returns an argument from index and reports an error if it is out of bounds
arg get_arg_from_index(format_context *f, s64 index) {
    if (index < f->Args.Count) {
        return get_arg(f->Args, index);
    }
    on_error(f, "Argument index out of range");
    return {};
}

struct width_checker {
    format_context *F;

    template <typename T>
    u32 operator()(T value) {
        if constexpr (types::is_integral<T>) {
            if (sign_bit(value)) {
                on_error(F, "Negative width");
                return (u32) -1;
            } else if ((u64) value > numeric_info<s32>::max()) {
                on_error(F, "Width value is too big");
                return (u32) -1;
            }
            return (u32) value;
        } else {
            on_error(F, "Width was not an integer");
            return (u32) -1;
        }
    }
};

struct precision_checker {
    format_context *F;

    template <typename T>
    s32 operator()(T value) {
        if constexpr (types::is_integral<T>) {
            if (sign_bit(value)) {
                on_error(F, "Negative precision");
                return -1;
            } else if ((u64) value > numeric_info<s32>::max()) {
                on_error(F, "Precision value is too big");
                return -1;
            }
            return (s32) value;
        } else {
            on_error(F, "Precision was not an integer");
            return -1;
        }
    }
};

bool handle_dynamic_specs(format_context *f) {
    assert(f->Specs);

    if (f->Specs->WidthIndex != -1) {
        auto width = get_arg_from_index(f, f->Specs->WidthIndex);
        if (width.Type != type::NONE) {
            f->Specs->Width = visit_fmt_arg(width_checker{f}, width);
            if (f->Specs->Width == (u32) -1) return false;
        }
    }
    if (f->Specs->PrecisionIndex != -1) {
        auto precision = get_arg_from_index(f, f->Specs->PrecisionIndex);
        if (precision.Type != type::NONE) {
            f->Specs->Precision = visit_fmt_arg(precision_checker{f}, precision);
            if (f->Specs->Precision == numeric_info<s32>::min()) return false;
        }
    }

    return true;
}

void parse_fmt_string(const string &fmtString, format_context *f) {
    parse_context *p = &f->Parse;

    auto write_until = [&](const utf8 *end) {
        if (!p->It.Count) return;
        while (true) {
            auto searchString = string(p->It.Data, end - p->It.Data);

            s64 bracket = find_cp(searchString, '}');
            if (bracket == -1) {
                write_no_specs(f, p->It.Data, end - p->It.Data);
                return;
            }

            auto *pbracket = get_cp_at_index(searchString.Data, searchString.Length, bracket);
            if (*(pbracket + 1) != '}') {
                on_error(f, "Unmatched \"}\" in format string - if you want to print it use \"}}\" to escape", pbracket - f->Parse.FormatString.Data);
                return;
            }

            write_no_specs(f, p->It.Data, pbracket - p->It.Data);
            write_no_specs(f, "}");

            s64 advance = pbracket + 2 - p->It.Data;
            p->It.Data += advance, p->It.Count -= advance;
        }
    };

    arg currentArg;

    while (p->It.Count) {
        s64 bracket = find_cp(p->It, '{');
        if (bracket == -1) {
            write_until(p->It.Data + p->It.Count);
            return;
        }

        auto *pbracket = get_cp_at_index(p->It.Data, p->It.Length, bracket);
        write_until(pbracket);

        s64 advance = pbracket + 1 - p->It.Data;
        p->It.Data += advance, p->It.Count -= advance;

        if (!p->It.Count) {
            on_error(f, "Invalid format string");
            return;
        }
        if (p->It[0] == '}') {
            // Implicit {} means "get the next argument"
            currentArg = get_arg_from_index(f, next_arg_id(p));
            if (currentArg.Type == type::NONE) return;  // The error was reported in _f->get_arg_from_ref_

            visit_fmt_arg(internal::format_context_visitor(f), currentArg);
        } else if (p->It[0] == '{') {
            // {{ means we escaped a {.
            write_until(p->It.Data + 1);
        } else if (p->It[0] == '!') {
            ++p->It.Data, --p->It.Count;  // Skip the !

            text_style style = {};
            bool success = parse_text_style(p, &style);
            if (!success) return;
            if (!p->It.Count || p->It[0] != '}') {
                on_error(f, "\"}\" expected");
                return;
            }

            if (!Context.FmtDisableAnsiCodes) {
                utf8 ansiBuffer[7 + 3 * 4 + 1];
                auto *ansiEnd = internal::color_to_ansi(ansiBuffer, style);
                write_no_specs(f, ansiBuffer, ansiEnd - ansiBuffer);

                u8 emphasis = (u8) style.Emphasis;
                if (emphasis) {
                    assert(!style.Background);
                    ansiEnd = internal::emphasis_to_ansi(ansiBuffer, emphasis);
                    write_no_specs(f, ansiBuffer, ansiEnd - ansiBuffer);
                }
            }
        } else {
            // Parse integer specified or a named argument
            s64 argId = parse_arg_id(p);
            if (argId == -1) return;

            currentArg = get_arg_from_index(f, argId);
            if (currentArg.Type == type::NONE) return;  // The error was reported in _f->get_arg_from_ref_

            utf8 c = p->It.Count ? p->It[0] : 0;
            if (c == '}') {
                visit_fmt_arg(internal::format_context_visitor(f), currentArg);
            } else if (c == ':') {
                ++p->It.Data, --p->It.Count;  // Skip the :

                dynamic_format_specs specs = {};
                bool success = parse_fmt_specs(p, currentArg.Type, &specs);
                if (!success) return;
                if (!p->It.Count || p->It[0] != '}') {
                    on_error(f, "\"}\" expected");
                    return;
                }

                f->Specs = &specs;
                success = handle_dynamic_specs(f);
                if (!success) return;

                visit_fmt_arg(internal::format_context_visitor(f), currentArg);

                f->Specs = null;
            } else {
                on_error(f, "\"}\" expected");
                return;
            }
        }
        ++p->It.Data, --p->It.Count;  // Go to the next byte
    }
}
}  // namespace fmt

LSTD_END_NAMESPACE
