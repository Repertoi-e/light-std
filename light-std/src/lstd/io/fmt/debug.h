#pragma once

#include "arg.h"

#include "../../storage/array.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct debug_struct_helper {
    struct field_entry {
        string Name;
        arg Arg;
    };

    format_context *F;
    string Name;
    array<field_entry> Fields;

    debug_struct_helper(format_context *f, string name) : F(f), Name(name) {}

    template <typename T>
    debug_struct_helper *field(string name, const T &val) {
        Fields.append({name, make_arg(val)});
        return this;
    }

    void finish();

   private:
    void write_field(field_entry *entry);
};

struct debug_tuple_helper {
    format_context *F;
    string Name;
    array<arg> Fields;

    debug_tuple_helper(format_context *f, string name) : F(f), Name(name) {}

    template <typename T>
    debug_tuple_helper *field(const T &val) {
        Fields.append(make_arg(val));
        return this;
    }

    void finish();
};

struct debug_list_helper {
    format_context *F;
    array<arg> Fields;

    debug_list_helper(format_context *f) : F(f) {}

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

    void finish();
};

}  // namespace fmt

LSTD_END_NAMESPACE
