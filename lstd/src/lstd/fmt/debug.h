#pragma once

#include "../memory/array.h"
#include "arg.h"

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
    bool NoSpecs;  // Write the result without taking into account specs for individual arguments

    debug_struct_helper(format_context *f, const string &name, bool noSpecs) : F(f), Name(name), NoSpecs(noSpecs) {}

    // I know we are against hidden freeing but having this destructor is actually really fine.
    // Things would be a whole more ugly and complicated without it.
    ~debug_struct_helper() { Fields.release(); }

    template <typename T>
    debug_struct_helper *field(const string &name, const T &val) {
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
    bool NoSpecs;  // Write the result without taking into account specs for individual arguments

    debug_tuple_helper(format_context *f, const string &name, bool noSpecs) : F(f), Name(name), NoSpecs(noSpecs) {}

    // I know we are against hidden freeing but having this destructor is actually really fine.
    // Things would be a whole more ugly and complicated without it.
    ~debug_tuple_helper() { Fields.release(); }

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
    bool NoSpecs;  // Write the result without taking into account specs for individual arguments

    debug_list_helper(format_context *f, bool noSpecs) : F(f), NoSpecs(noSpecs) {}

    // I know we are against hidden freeing but having this destructor is actually really fine.
    // Things would be a whole more ugly and complicated without it.
    ~debug_list_helper() { Fields.release(); }

    template <typename T>
    debug_list_helper *entries(const array<T> &val) {
        auto *p = val.Data;
        s64 n = val.Count;
        while (n) {
            Fields.append(make_arg((T &&) *p));
            ++p, --n;
        }
        return this;
    }

    template <typename T>
    debug_list_helper *entries(T *begin, T *end) {
        return entries(array<T>(begin, end - begin));
    }

    template <typename T>
    debug_list_helper *entries(T *begin, s64 count) {
        return entries(array<T>(begin, count));
    }

    void finish();
};

}  // namespace fmt

LSTD_END_NAMESPACE
