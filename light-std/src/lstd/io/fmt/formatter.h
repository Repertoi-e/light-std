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
        visit_fmt_arg([&](auto value) { f->write(value); }, make_arg(src));
    };
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
            auto visitor = internal::format_context_visitor{Context};
            visit_fmt_arg(visitor, Arg);
        }
    }

    const byte *on_format_specs(const byte *begin, const byte *end) {
        Context->Parse.advance_to(begin);
        Context->HasSpecsForCurrent = true;
        Context->Specs = {};

        format_specs specs;
        internal::specs_checker<internal::dynamic_specs_handler> handler(
            internal::dynamic_specs_handler{&Context->Parse, &Context->Specs}, Context->Parse.ErrorHandlerFunc,
            Context->Parse.get_error_context(), Arg.Type);
        begin = parse_fmt_specs(begin, end, &handler, &Context->Parse);

        Context->Parse.advance_to(begin);
        if (begin == end || *begin != '}') {
            Context->on_error("Missing '}' in format string");
        }

        internal::handle_dynamic_specs(Context);

        auto visitor = internal::format_context_visitor{Context};
        visit_fmt_arg(visitor, Arg);
        return begin;
    }

    void on_error(const byte *message) { Context->on_error(message); }
};
}  // namespace fmt

LSTD_END_NAMESPACE
