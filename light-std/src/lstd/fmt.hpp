#pragma once

#include "memory/temporary_allocator.hpp"
#include "memory/dynamic_array.hpp"

#include "format/console_colors.hpp"
#include "format/core.hpp"
#include "format/parse.hpp"

LSTD_BEGIN_NAMESPACE

namespace fmt {

// Specialize Formatter for non-custom types
template <typename T>
struct Formatter<T, typename std::enable_if_t<Get_Type<T>::Value != Format_Type::CUSTOM>> {
    void format(const T &value, Format_Context &f) { f.write_argument(make_argument(value)); }
};

template <>
struct Formatter<String_Builder> {
    void format(const String_Builder &value, Format_Context &f) {
        value.traverse([&](const string_view &view) { f.write(view); });
    }
};

template <typename T, size_t Size>
struct Formatter<Array<T, Size>> {
    void format(const Array<T, Size> &value, Format_Context &f) {
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
struct Formatter<Dynamic_Array<T>> {
    void format(const Dynamic_Array<T> &value, Format_Context &f) {
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

namespace internal {
inline void do_formatting(Format_Context &context) {
    Argument arg;

    auto &specs = context.ParseContext.Specs;

    const byte *&it = context.ParseContext.It;
    const byte *end = (const byte *) context.ParseContext.FormatString.end().to_pointer();
    while (it != end) {
        size_t curly = Memory_View(it, end - it).find('{');
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

            auto error = internal::parse_arg_id(p, internal::ID_Adapter(context, arg));
            if (error != Parsing_Error_Code::NONE) {
                context.Out.append_cstring("{Invalid format string}");
                return;
            }
            it = p;
            char32_t c = p != end ? *p : 0;
            if (c == '}') {
                context.write_argument(arg);
            } else if (c == ':') {
                it = ++p;
                auto error = parse_and_validate_specs(arg.Type, context);
                if (error != Parsing_Error_Code::NONE) {
                    context.Out.append_cstring("{");
                    context.Out.append(get_message_from_parsing_error_code(error));
                    context.Out.append_cstring("}");
                    return;
                }
                p = it;
                if (*it == '}') {
                    context.write_argument(arg);
                } else {
                    context.Out.append_cstring("{Unknown format specifier}");
                    return;
                }
            } else {
                context.Out.append_cstring("{Missing \"}\" in format string}");
                return;
            }
        }
        it = p + 1;
    }
}

template <typename... Args>
void to_writer(io::Writer &writer, const string_view &formatString, Args &&... args) {
    Arguments_Array<Args...> store = {args...};
    auto context = Format_Context(writer, formatString, Arguments(store));
    internal::do_formatting(context);
    context.flush();
}
}  // namespace internal

template <typename... Args>
string sprint(const string_view &formatString, Args &&... args) {
    Arguments_Array<Args...> store = {args...};

    io::String_Writer writer;
    writer.write_fmt(formatString, std::forward<Args>(args)...);
    return writer.Builder.combine();
}

template <typename... Args>
string tprint(const string_view &formatString, Args &&... args) {
    auto tempContext = Context;
    tempContext.Allocator = TEMPORARY_ALLOC;

    string result;
    result.Allocator = TEMPORARY_ALLOC;
    PUSH_CONTEXT(tempContext) { result = sprint(formatString, std::forward<Args>(args)...); }
    return result;
}

template <typename... Args>
void print(const string_view &formatString, Args &&... args) {
    internal::to_writer(*Context.Log, formatString, std::forward<Args>(args)...);
    Context.Log->flush();
}

template <typename T>
inline string to_string(const T &value) {
    return sprint("{}", value);
}

}  // namespace fmt

namespace fmt {

// Format a string using the temporary allocator
template <typename... Args>
inline string tprint(const string &formatString, Args &&... args) {
    assert(TemporaryAllocatorData);

    auto tempContext = Context;
    tempContext.Allocator = TEMPORARY_ALLOC;
    PUSH_CONTEXT(tempContext) { return sprint(formatString, std::forward<Args>(args)...); }
}
}  // namespace fmt

LSTD_END_NAMESPACE
