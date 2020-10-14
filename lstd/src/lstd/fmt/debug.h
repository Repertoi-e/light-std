#pragma once

#include "../memory/array.h"
#include "arg.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct format_struct_helper {
    struct field_entry {
        string Name;
        arg Arg;
    };

    format_context *F;
    string Name;
    array<field_entry> Fields;
    bool NoSpecs;  // Write the result without taking into account specs for individual arguments

    format_struct_helper(format_context *f, const string &name, bool noSpecs) : F(f), Name(name), NoSpecs(noSpecs) {}

    // I know we are against hidden freeing but having this destructor is actually really fine.
    // Things would be a whole more ugly and complicated without it.
    ~format_struct_helper() { free(Fields); }

    template <typename T>
    format_struct_helper *field(const string &name, const T &value) {
        append(Fields, {name, make_arg(value)});
        return this;
    }

    void finish();

   private:
    void write_field(field_entry *entry);
};

struct format_tuple_helper {
    format_context *F;
    string Name;
    array<arg> Fields;
    bool NoSpecs;  // Write the result without taking into account specs for individual arguments

    format_tuple_helper(format_context *f, const string &name, bool noSpecs) : F(f), Name(name), NoSpecs(noSpecs) {}

    // I know we are against hidden freeing but having this destructor is actually really fine.
    // Things would be a whole more ugly and complicated without it.
    ~format_tuple_helper() { free(Fields); }

    template <typename T>
    format_tuple_helper *field(const T &value) {
        append(Fields, make_arg(value));
        return this;
    }

    void finish();
};

struct format_list_helper {
    format_context *F;
    array<arg> Fields;
    bool NoSpecs;  // Write the result without taking into account specs for individual arguments

    format_list_helper(format_context *f, bool noSpecs) : F(f), NoSpecs(noSpecs) {}

    // I know we are against hidden freeing but having this destructor is actually really fine.
    // Things would be a whole more ugly and complicated without it.
    ~format_list_helper() { free(Fields); }

    template <typename T>
    format_list_helper *entries(const array_view<T> &values) {
        For(values) append(Fields, make_arg(it));
        return this;
    }

    template <typename T>
    format_list_helper *entries(T *begin, T *end) {
        return entries(array_view<T>(begin, end - begin));
    }

    template <typename T>
    format_list_helper *entries(T *begin, s64 count) {
        return entries(array_view<T>(begin, count));
    }

    void finish();
};

}  // namespace fmt

LSTD_END_NAMESPACE
