#pragma once

#include "memory/dynamic_array.hpp"
#include "memory/temporary_allocator.hpp"

#include "thread.hpp"

#include "file/path.hpp"

#include "format/console_colors.hpp"
#include "format/core.hpp"
#include "format/parse.hpp"

LSTD_BEGIN_NAMESPACE

namespace fmt {

// Specialize Formatter for non-custom types
template <typename T>
struct formatter<T, std::enable_if_t<get_type<T>::Value != format_type::CUSTOM>> {
    void format(const T &value, format_context &f) { f.write_argument(make_argument(value)); }
};

template <>
struct formatter<string_builder> {
    void format(const string_builder &value, format_context &f) {
        value.traverse([&](const string_view &view) { f.write(view); });
    }
};

template <typename T, size_t Size>
struct formatter<array<T, Size>> {
    void format(const array<T, Size> &value, format_context &f) {
        f.write("{ [");
        if (value.Count > 0) {
            f.write_argument(make_argument(value[0]));
            For(range(1, value.Count)) {
                f.write(", ");
                f.write_argument(make_argument(value[it]));
            }
        }
        f.write_fmt("], Count: {} ", value.Count);
        f.write("}");
    }
};

template <typename T>
struct formatter<dynamic_array<T>> {
    void format(const dynamic_array<T> &value, format_context &f) {
        f.write("{ [");
        if (value.Count > 0) {
            f.write_argument(fmt::make_argument(value[0]));
            For(range(1, value.Count)) {
                f.write(", ");
                f.write_argument(make_argument(value[it]));
            }
        }
        f.write_fmt("], Count: {} ", value.Count);
        f.write("}");
    }
};

template <>
struct formatter<thread::thread::id> {
    void format(const thread::thread::id &value, format_context &f) const { f.write_int(value.Value); }
};

template <>
struct formatter<file::path> {
    void format(const file::path &value, format_context &f) const { f.write(value.get()); }
};

namespace internal {
inline void do_formatting(format_context &context) {
    argument arg;

    auto &specs = context.ParseContext.Specs;

    const byte *&it = context.ParseContext.It;
    const byte *end = (const byte *) context.ParseContext.FormatString.end().to_pointer();
    while (it != end) {
        size_t curly = memory_view(it, end - it).find('{');
        if (*it != '{' && curly == npos) {
            context.Out.append(string_view(it, end - it));
            return;
        }
        auto p = it + curly;
        context.Out.append(string_view(it, p - it));
        ++p;
        if (p == end) {
            assert(false && "Invalid format string");
            return;
        }

        if (*p == '}') {
            arg = context.next_arg();

            it = p;
            specs = {};
            context.write_argument(arg);
        } else if (*p == '{') {
            context.Out.append(string_view(p, 1));
        } else {
            specs = {};

            auto error = parse_arg_id(p, id_adapter(context, arg));
            if (error != parsing_error_code::NONE) {
                context.Out.append("{Invalid format string}");
                return;
            }
            it = p;
            char32_t c = p != end ? *p : 0;
            if (c == '}') {
                context.write_argument(arg);
            } else if (c == ':') {
                it = ++p;
                auto error = parse_and_validate_specs(arg.Type, context);
                if (error != parsing_error_code::NONE) {
                    context.Out.append("{");
                    context.Out.append(get_message_from_parsing_error_code(error));
                    context.Out.append("}");
                    return;
                }
                p = it;
                if (*it == '}') {
                    context.write_argument(arg);
                } else {
                    context.Out.append("{Unknown format specifier}");
                    return;
                }
            } else {
                context.Out.append("{Missing \"}\" in format string}");
                return;
            }
        }
        it = p + 1;
    }
}

template <typename... Args>
void to_writer(io::writer &writer, const string_view &formatString, Args &&... args) {
    arguments_array<Args...> store = {args...};
    auto context = format_context(writer, formatString, arguments(store));
    do_formatting(context);
    context.flush();
}
}  // namespace internal

template <typename... Args>
string sprint(const string_view &formatString, Args &&... args) {
    arguments_array<Args...> store = {args...};

    io::string_writer writer;
    writer.write_fmt(formatString, std::forward<Args>(args)...);
    return writer.Builder.combine();
}

template <typename... Args>
string tprint(const string_view &formatString, Args &&... args) {
    string result;
    result.Allocator = TEMPORARY_ALLOC;
    PUSH_CONTEXT(Allocator, TEMPORARY_ALLOC) { result = sprint(formatString, std::forward<Args>(args)...); }
    return result;
}

template <typename... Args>
void print(const string_view &formatString, Args &&... args) {
    internal::to_writer(*Context.Log, formatString, std::forward<Args>(args)...);
    Context.Log->flush();
}

template <typename T>
string to_string(const T &value) {
    return sprint("{}", value);
}

}  // namespace fmt

namespace fmt {

// Format a string using the temporary allocator
template <typename... Args>
string tprint(const string &formatString, Args &&... args) {
    assert(g_TemporaryAllocatorData);

    PUSH_CONTEXT(Allocator, TEMPORARY_ALLOC) { return sprint(formatString, std::forward<Args>(args)...); }
}
}  // namespace fmt

LSTD_END_NAMESPACE
