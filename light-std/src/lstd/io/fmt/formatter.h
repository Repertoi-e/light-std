#pragma once

#include "format_context.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

namespace internal {
struct width_checker {
    format_context *Context;

    width_checker(format_context *context) : Context(context) {}

    template <typename T>
    enable_if_t<is_integer_v<T>, u64> operator()(T value) {
        if (IS_NEG(value)) Context->on_error("Negative width");
        return (u64) value;
    }

    template <typename T>
    enable_if_t<!is_integer_v<T>, u64> operator()(T) {
        Context->on_error("Width is not integer");
        return 0;
    }
};

struct precision_checker {
    format_context *Context;

    precision_checker(format_context *context) : Context(context) {}

    template <typename T>
    enable_if_t<is_integer_v<T>, u64> operator()(T value) {
        if (IS_NEG(value)) Context->on_error("Negative precision");
        return (u64) value;
    }

    template <typename T>
    enable_if_t<!is_integer_v<T>, u64> operator()(T) {
        Context->on_error("Precision is not integer");
        return 0;
    }
};

inline void handle_dynamic_specs(format_context *f) {
    auto *specs = &f->Specs;

    if (specs->WidthRef.Kind != arg_ref::kind::NONE) {
        arg target;
        if (specs->WidthRef.Kind == arg_ref::kind::INDEX) {
            target = f->get_arg(specs->WidthRef.Index);
        } else {
            auto *originalBegin = f->Parse.get_original_fmt_string().begin().to_pointer();
            auto name = specs->WidthRef.Name.to_view(originalBegin);
            target = f->get_arg(name);
        }

        auto value = visit_fmt_arg(width_checker{f}, target);
        if (value > numeric_info<s32>::max()) {
            f->on_error("Number is too big");
        } else {
            specs->Width = (u32) value;
        }
    }

    if (specs->PrecisionRef.Kind != arg_ref::kind::NONE) {
        arg target;
        if (specs->PrecisionRef.Kind == arg_ref::kind::INDEX) {
            target = f->get_arg(specs->PrecisionRef.Index);
        } else {
            auto *originalBegin = f->Parse.get_original_fmt_string().begin().to_pointer();
            auto name = specs->PrecisionRef.Name.to_view(originalBegin);
            target = f->get_arg(name);
        }
        auto value = visit_fmt_arg(precision_checker{f}, target);
        if (value > numeric_info<s32>::max()) {
            f->on_error("Number is too big");
        } else {
            specs->Precision = (s32) value;
        }
    }
}
}  // namespace internal

template <typename T>
struct formatter<T, enable_if_t<(type) type_constant_v<T> != type::CUSTOM>> {
    void format(T src, format_context *f) {
        internal::handle_dynamic_specs(f);
        visit_fmt_arg(internal::format_context_visitor(f), make_arg(src));
    }
};

struct format_handler {
    format_context *Context;
    arg Arg;

    format_handler(format_context *context) : Context(context) {}

    void on_text(const byte *begin, const byte *end) { Context->write_no_specs(begin, end - begin); }

    void get_arg(u32 id) { Arg = Context->get_arg(id); }

    void on_arg_id() { get_arg(Context->Parse.next_arg_id()); }
    void on_arg_id(u32 id) {
        Context->Parse.check_arg_id(id);
        get_arg(id);
    }
    void on_arg_id(string_view name) { Arg = Context->get_arg(name); }

    void on_replacement_field(const byte *p) {
        Context->Parse.advance_to(p);
        Context->HasSpecsForCurrent = false;

        if (Arg.Type == type::CUSTOM) {
            auto handle = typename arg::handle(Arg.Value.Custom);
            handle.format(Context);
        } else {
            visit_fmt_arg(internal::format_context_visitor(Context), Arg);
        }
    }

    const byte *on_format_specs(const byte *begin, const byte *end) {
        Context->Parse.advance_to(begin);
        Context->HasSpecsForCurrent = true;
        Context->Specs = {};

        format_specs specs;
        internal::specs_checker handler(internal::dynamic_specs_handler{&Context->Parse, &Context->Specs},
                                        Context->Parse.ErrorHandlerFunc, Context->Parse.get_error_context(), Arg.Type);
        begin = parse_fmt_specs(begin, end, &handler, &Context->Parse);

        Context->Parse.advance_to(begin);
        if (begin == end || *begin != '}') {
            Context->on_error("Missing '}' in format string");
        }

        internal::handle_dynamic_specs(Context);

        visit_fmt_arg(internal::format_context_visitor(Context), Arg);
        return begin;
    }

    const byte *on_text_style(const byte *begin, const byte *end) {
        auto textStyle = parse_text_style(&begin, end, this);
        Context->Parse.advance_to(begin);

        byte buffer[7 + 3 * 4 + 1];
        byte *p = buffer;
        if (textStyle.ColorKind != text_style::color_kind::NONE) {
            if (textStyle.ColorKind == text_style::color_kind::TERMINAL) {
                // Background terminal colors are 10 more than the foreground ones
                u32 value = (u32) textStyle.Color.Terminal + (textStyle.Background ? 10 : 0);

                *p++ = '\x1b';
                *p++ = '[';

                if (value >= 100) {
                    *p++ = '1';
                    value %= 100;
                }
                *p++ = '0' + value / 10;
                *p++ = '0' + value % 10;

                *p++ = 'm';
                *p++ = '\0';
            } else {
                if (textStyle.Background) {
                    copy_memory(p, internal::BG_COLOR, 7);
                } else {
                    copy_memory(p, internal::FG_COLOR, 7);
                }

                u8 r = (u8)((textStyle.Color.RGB >> 16) & 0xFF);
                u8 g = (u8)((textStyle.Color.RGB >> 8) & 0xFF);
                u8 b = (u8)((textStyle.Color.RGB) & 0xFF);
                to_esc(r, buffer + 7, ';');
                to_esc(g, buffer + 11, ';');
                to_esc(b, buffer + 15, 'm');
                p = buffer + 19;
            }
        } else if ((u8) textStyle.Emphasis == 0) {
            // Empty text style spec means "reset"
            auto size = cstring_strlen(internal::RESET_COLOR);
            copy_memory(p, internal::RESET_COLOR, size);
            p += size;
        }
        Context->write_no_specs(buffer, p - buffer);

        u8 emBits = (u8) textStyle.Emphasis;
        if (emBits) {
            assert(!textStyle.Background);

            u8 codes[4] = {};
            if (emBits & (u8) emphasis::BOLD) codes[0] = 1;
            if (emBits & (u8) emphasis::ITALIC) codes[1] = 3;
            if (emBits & (u8) emphasis::UNDERLINE) codes[2] = 4;
            if (emBits & (u8) emphasis::STRIKETHROUGH) codes[3] = 9;

            p = buffer;
            For(range(4)) {
                if (!codes[it]) continue;

                *p++ = '\x1b';
                *p++ = '[';
                *p++ = '0' + codes[it];
                *p++ = 'm';
            }
        }
        Context->write_no_specs(buffer, p - buffer);

        return begin;
    }

    void on_error(const byte *message) { Context->on_error(message); }

   private:
    // Used when making ANSI escape codes for text styles
    void to_esc(u8 c, byte *out, byte delimiter) {
        out[0] = '0' + c / 100;
        out[1] = '0' + c / 10 % 10;
        out[2] = '0' + c % 10;
        out[3] = delimiter;
    }
};
}  // namespace fmt

LSTD_END_NAMESPACE
