#pragma once

#include "arg.h"

#include "../../storage/array.h"
#include "../../storage/string.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct format_context;

struct debug_struct_field_entry {
    string Name;
    arg Arg;
};

// Kill me now, kill me fast
// Kill me now, kill me fast
// Kill me now, kill me fast

}  // namespace fmt

LSTD_END_NAMESPACE

// :ExplicitDeclareIsPod
DECLARE_IS_POD(fmt::debug_struct_field_entry, true)

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct debug_struct_helper {
    format_context *Context;
    string Name;
    array<debug_struct_field_entry> Fields;

    debug_struct_helper(format_context *context, string name) : Context(context), Name(name) {}

    template <typename T>
    debug_struct_helper *field(string name, const T &val) {
        Fields.append({name, make_arg(val)});
        return this;
    }

    // Defined after _format_context_, because C++
    void finish();
    void write_field(debug_struct_field_entry *entry);
};

struct debug_tuple_helper {
    format_context *Context;
    string Name;
    array<arg> Fields;

    debug_tuple_helper(format_context *context, string name) : Context(context), Name(name) {}

    template <typename T>
    debug_tuple_helper *field(const T &val) {
        Fields.append(make_arg(val));
        return this;
    }

    // Defined after _format_context_, because C++
    void finish();
};

struct debug_list_helper {
    format_context *Context;
    array<arg> Fields;

    debug_list_helper(format_context *context) : Context(context) {}

    template <typename T>
    debug_list_helper *entries(array_view<T> val) {
        For(val) Fields.append(make_arg(it));
        return this;
    }

    template <typename T>
    debug_list_helper *entries(T *begin, T *end) {
        return entries(array_view<T>(begin, end));
    }

    template <typename T>
    debug_list_helper *entries(T *begin, size_t count) {
        return entries(array_view<T>(begin, begin + count));
    }

    // Defined after _format_context_, because C++
    void finish();
};

}  // namespace fmt

LSTD_END_NAMESPACE
