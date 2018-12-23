#pragma once

#include "console_colors.h"
#include "core.h"
#include "parse.h"

CPPU_BEGIN_NAMESPACE

namespace fmt {

// Specialize Formatter for non-custom types
template <typename T>
struct Formatter<T, typename std::enable_if_t<Get_Type<T>::Value != Format_Type::CUSTOM>> {
    void format(const T &value, Format_Context &f) { f.write_argument(make_argument(value)); }
};

template <>
struct Formatter<String_Builder> {
    void format(const String_Builder &value, Format_Context &f) { f.write(string_view(value.combine())); }
};

template <typename T, size_t Size>
struct Formatter<Array<T, Size>> {
    void format(const Array<T, Size> &value, Format_Context &f) {
        f.write("{ [");
        if (value.Count > 0) {
            f.write_argument(make_argument(value[0]));
            for (s32 i : range(1, value.Count)) {
                f.write(", ");
                f.write_argument(make_argument(value[i]));
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
            for (s32 i : range(1, value.Count)) {
                f.write(", ");
                f.write_argument(fmt::make_argument(value[i]));
            }
        }
        f.write("], ");
        f.write_fmt("Count: {} ", value.Count);
        f.write("}");
    }
};

namespace internal {
inline void helper_write(String_Builder &builder, string_view::iterator begin, const string_view::iterator &end) {
    if (begin == end) return;
    while (true) {
        size_t curly = string_view(begin.to_pointer(), end - begin).find('}');
        if (curly == npos) {
            builder.append_pointer_and_size(begin.to_pointer(), end - begin);
            return;
        }
        auto p = begin + curly;
        if (p == end || *p != '}') {
            assert(false && "unmatched } in format string");
            return;
        }
        builder.append_pointer_and_size(begin.to_pointer(), p - begin);
        begin = p + 1;
    }
}
}  // namespace internal

template <typename... Args>
string sprint(const string_view &formatString, Args &&... args) {
    Arguments_Array<Args...> store = {args...};

    Format_Context context(formatString, Arguments(store));
    Argument arg;

    auto &specs = context.ParseContext.Specs;

    auto &it = context.ParseContext.It;
    auto end = context.ParseContext.FormatString.end();
    while (it != end) {
        size_t curly = string_view(it.to_pointer(), end - it).find('{');
        if (*it != '{' && curly == npos) {
            internal::helper_write(context.Out, it, end);
            return context.Out.combine();
        }
        auto p = it + curly;
        internal::helper_write(context.Out, it, p);
        ++p;
        if (p == end) {
            assert(false && "Invalid format string");
            return context.Out.combine();
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
                context.Out.append("{Invalid format string}");
                return context.Out.combine();
            }
            it = p;
            char32_t c = p != end ? *p : 0;
            if (c == '}') {
                context.write_argument(arg);
            } else if (c == ':') {
                it = ++p;
                auto error = parse_and_validate_specs(arg.Type, context);
                if (error != Parsing_Error_Code::NONE) {
                    context.Out.append(get_message_from_parsing_error_code(error));
                    return context.Out.combine();
                }
                p = it;
                if (*it == '}') {
                    context.write_argument(arg);
                } else {
                    context.Out.append("{Unknown format specifier}");
                    return context.Out.combine();
                }
            } else {
                context.Out.append("{Missing \"}\" in format string}");
                return context.Out.combine();
            }
        }
        it = p + 1;
    }

    return context.Out.combine();
}

template <typename... Args>
void tprint(const string_view &formatString, Args &&... args) {
    auto tempContext = __context;
    tempContext.Allocator = TEMPORARY_ALLOC;

    string result;
    result.Allocator = TEMPORARY_ALLOC;
    PUSH_CONTEXT(tempContext) { result = sprint(formatString, std::forward<Args>(args)...); }
    return result;
}

template <typename... Args>
void print(const string_view &formatString, Args &&... args) {
    // TODO: A way to optimize this would be by directly outputting text to
    // the console instead of sprinting it to a buffer first and then outputting it.
    print_string_to_console(string_view(sprint(formatString, std::forward<Args>(args)...)));
}

template <typename T>
inline string to_string(const T &value) {
    // TODO: Speed...
    return sprint("{}", value);
}

}  // namespace fmt

#include "../memory/temporary_allocator.h"

namespace fmt {

// Format a string using the temporary allocator
template <typename... Args>
inline string tprint(const string &formatString, Args &&... args) {
    assert(__temporary_allocator_data);

    Allocator_Closure oldAllocator;
    oldAllocator = __context.Allocator;
    __context.Allocator = {__temporary_allocator, __temporary_allocator_data};

    auto result = sprint(formatString, std::forward<Args>(args)...);

    __context.Allocator = oldAllocator;

    return result;
}
}  // namespace fmt

CPPU_END_NAMESPACE
