#pragma once

#include "console_colors.hpp"
#include "core.hpp"
#include "parse.hpp"

CPPU_BEGIN_NAMESPACE

namespace fmt {

// Specialize Formatter for non-custom types
template <typename T>
struct Formatter<T, typename std::enable_if_t<Get_Type<T>::Value != Format_Type::CUSTOM>> {
    void format(const T &value, Format_Context &f) { f.write_argument(make_argument(value)); }
};

template <>
struct Formatter<String_Builder> {
    void format(const String_Builder &value, Format_Context &f) { f.write(value.combine().get_view()); }
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
        f.write("], ");
        f.write_fmt("Count: {} ", value.Count);
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
                f.write_argument(fmt::make_argument(value[it]));
            }
        }
        f.write("], ");
        f.write_fmt("Count: {} ", value.Count);
        f.write("}");
    }
};

namespace internal {
inline void helper_write(io::Writer &out, string_view::iterator begin, const string_view::iterator &end) {
    if (begin == end) return;
    while (true) {
        size_t curly = string_view(begin.to_pointer(), end - begin).find('}');
        if (curly == npos) {
            out.write(begin.to_pointer(), end - begin);
            return;
        }
        auto p = begin + curly;
        if (p == end || *p != '}') {
            assert(false && "unmatched } in format string");
            return;
        }
        out.write(begin.to_pointer(), p - begin);
        begin = p + 1;
    }
}

inline void do_formatting(Format_Context &context) {
    Argument arg;

    auto &specs = context.ParseContext.Specs;

    auto &it = context.ParseContext.It;
    auto end = context.ParseContext.FormatString.end();
    while (it != end) {
        size_t curly = string_view(it.to_pointer(), end - it).find('{');
        if (*it != '{' && curly == npos) {
            internal::helper_write(context.Out, it, end);
            return;
        }
        auto p = it + curly;
        internal::helper_write(context.Out, it, p);
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
            internal::helper_write(context.Out, p, p + 1);
        } else {
            specs = {};

            auto error = internal::parse_arg_id(p, internal::ID_Adapter(context, arg));
            if (error != Parsing_Error_Code::NONE) {
                context.Out.write("{Invalid format string}");
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
                    context.Out.write('{');
                    context.Out.write(get_message_from_parsing_error_code(error));
                    context.Out.write('}');
                    return;
                }
                p = it;
                if (*it == '}') {
                    context.write_argument(arg);
                } else {
                    context.Out.write("{Unknown format specifier}");
                    return;
                }
            } else {
                context.Out.write("{Missing \"}\" in format string}");
                return;
            }
        }
        it = p + 1;
    }
}

template <typename... Args>
void to_writer(io::Writer &writer, const string_view &formatString, Args &&... args) {
    Arguments_Array<Args...> store = {args...};
    internal::do_formatting(Format_Context(writer, formatString, Arguments(store)));
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
void tprint(const string_view &formatString, Args &&... args) {
    auto tempContext = Context;
    tempContext.Allocator = TEMPORARY_ALLOC;

    string result;
    result.Allocator = TEMPORARY_ALLOC;
    PUSH_CONTEXT(tempContext) { result = sprint(formatString, std::forward<Args>(args)...); }
    return result;
}

template <typename... Args>
void print(const string_view &formatString, Args &&... args) {
    Context.Log->write_fmt(formatString, std::forward<Args>(args)...);
}

template <typename T>
inline string to_string(const T &value) {
    return sprint("{}", value);
}

}  // namespace fmt

#include "../memory/temporary_allocator.hpp"

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

CPPU_END_NAMESPACE
