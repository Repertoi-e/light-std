#pragma once

#pragma once

#include "../console_writer.h"
#include "parse.h"
#include "value.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

// Supports constexpr
template <typename ConstexprErrorHandler, typename... Args>
struct fmt_string_checker {
    static constexpr size_t NUM_ARGS = sizeof...(Args);

    // Format specifier parsing function.
    using parse_func = const byte *(*) (parse_context *, dynamic_format_specs *specs);

    template <typename ConstexprErrorHandler>
    static constexpr void parse_context_error_handler(const byte *message, error_context errorContext) {
        ConstexprErrorHandler handler;
        handler(message, errorContext);
    }

    u32 ArgID = numeric_info<u32>::max();
    parse_context Parse;
    parse_func ParseFuncs[NUM_ARGS > 0 ? NUM_ARGS : 1];
    u32 ArgsEncountered = 0;
    ConstexprErrorHandler ErrorHandler;

    constexpr fmt_string_checker(string_view fmtString)
        : Parse(fmtString, parse_context_error_handler<ConstexprErrorHandler>), ParseFuncs{&parse_fmt_specs<Args>...} {}

    constexpr void on_text(const byte *, const byte *) {}

    constexpr void on_arg_id() {
        ArgID = Parse.next_arg_id();
        check_arg_id();
    }

    constexpr void on_arg_id(u32 id) {
        ArgID = id;
        Parse.check_arg_id(id);
        check_arg_id();
    }

    // @TODO: Let's support named arguments...
    constexpr void on_arg_id(string_view) { on_error("Constexpr checks don't support named arguments, yet"); }

    constexpr void on_replacement_field(const byte *) { ++ArgsEncountered; }

    constexpr const byte *on_format_specs(const byte *begin, const byte *) {
        ++ArgsEncountered;
        Parse.advance_to(begin);

        dynamic_format_specs specs;
        return ArgID < NUM_ARGS ? ParseFuncs[ArgID](&Parse, &specs) : begin;
    }

    constexpr const byte *on_text_style(const byte *begin, const byte *end) {
        Parse.advance_to(begin);
        parse_text_style(&begin, end, this);
        return begin;
    }

    // Error location gets filled in the parse context
    constexpr void on_error(const byte *message) { Parse.on_error(message); }

   private:
    constexpr void check_arg_id() {
        if (ArgID >= NUM_ARGS) on_error("Argument index out of range");
    }
};

namespace internal {
template <typename Handler>
struct id_adapter {
    Handler *IdHandler;

    constexpr id_adapter(Handler *idHandler) : IdHandler(idHandler) {}

    constexpr void operator()() { IdHandler->on_arg_id(); }
    constexpr void operator()(u32 id) { IdHandler->on_arg_id(id); }
    constexpr void operator()(string_view id) { IdHandler->on_arg_id(id); }
    constexpr void on_error(const byte *message) { IdHandler->on_error(message); }
};
}  // namespace internal

template <bool IS_CONSTEXPR, typename Handler>
constexpr void parse_format_string(parse_context *context, Handler *handler) {
    auto fmtString = context->FmtString;

    struct writer {
        constexpr void operator()(const byte *begin, const byte *end) {
            if (begin == end) return;
            while (true) {
                // @Performance string_view::find is a simple constexpr function (and thus - slower)
                auto view = string_view(begin, end - begin);
                auto index = view.find('}');
                if (index == npos) {
                    Handler->on_text(begin, end);
                    return;
                }

                ++index;

                auto byteIndex = get_byte_index_from_cp_index(view.Data, view.Length, index);
                if (index == (end - begin) || *(begin + byteIndex) != '}') {
                    Handler->on_error("Unmatched '}' in format string");
                    return;
                }
                Handler->on_text(begin, begin + byteIndex);
                begin += byteIndex + 1;
            }
        }
        Handler *Handler;
    } write{handler};

    auto *begin = fmtString.Data;
    auto *end = begin + fmtString.ByteLength;

    while (begin != end) {
        // Doing two passes with memchr (one for '{' and another for '}') is up to
        // 2.5x faster than the naive one-pass implementation on big format strings.
        const byte *p = begin;

        auto view = string_view(begin, end - begin);
        auto index = view.find('{');
        if (*begin != '{' && index == npos) {
            write(begin, end);
            return;
        }

        p += get_byte_index_from_cp_index(view.Data, view.Length, index);
        write(begin, p);
        ++p;

        if (p == end) {
            handler->on_error("Invalid format string");
            return;
        }
        if (*p == '}') {
            handler->on_arg_id();
            handler->on_replacement_field(p);
        } else if (*p == '{') {
            handler->on_text(p, p + 1);
        } else if (*p == '!') {
            p = handler->on_text_style(p + 1, end);
            if (p == end || *p != '}') return handler->on_error("Missing '}' in format string");
        } else {
            auto idHandler = internal::id_adapter<Handler>(handler);
            p = parse_arg_id(p, end, &idHandler);
            byte c = p != end ? *p : 0;
            if (c == '}') {
                handler->on_replacement_field(p);
            } else if (c == ':') {
                p = handler->on_format_specs(p + 1, end);
                if (p == end || *p != '}') return handler->on_error("Unknown format specifier");
            } else {
                return handler->on_error("Missing '}' in format string");
            }
        }
        begin = p + 1;
    }
}

namespace internal {
struct string_literal {};

#define FMT(fmtLiteral)                                                                             \
    [] {                                                                                            \
        struct str : fmt::internal::string_literal {                                                \
            constexpr operator string_view() const { return {fmtLiteral, sizeof(fmtLiteral) - 1}; } \
        } result;                                                                                   \
        (void) (string_view)(result);                                                               \
        return result;                                                                              \
    }()

struct default_constexpr_error_handler {
    constexpr void operator()(const byte *message, error_context errorContext) {}
};

template <typename... Args>
constexpr bool do_check_format_string(string_view fmtString) {
    fmt_string_checker<default_constexpr_error_handler, Args...> checker(fmtString);
    parse_format_string<true>(&checker.Parse, &checker);
    if (checker.ArgsEncountered < checker.NUM_ARGS) {
        checker.Parse.on_error("Format string contains more fields than given arguments.");
    }
    return true;
}
}  // namespace internal

template <typename T>
constexpr bool can_be_fmt_string_v = is_constructible_v<string_view, T>;

template <typename... Args, typename S>
enable_if_t<(is_base_of_v<internal::string_literal, S>)> maybe_compile_time_check_format_string(S fmtString) {
    constexpr bool result = internal::do_check_format_string<Args...>(string_view(fmtString));
    (void) result;
}

template <typename... Args, typename S>
enable_if_t<(!is_base_of_v<internal::string_literal, S>)> maybe_compile_time_check_format_string(S fmtString) {}

}  // namespace fmt

LSTD_END_NAMESPACE