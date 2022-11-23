#pragma once

#include "namespace.h"

// There is a quirk with modules name mangling which means we have to use a header file 
// just for this function to be visible everywhere and be able to be specialized.

LSTD_BEGIN_NAMESPACE

struct fmt_context;

// Specialize this function for formatting a custom type.
template <typename T>
void write_custom(fmt_context *f, const T *t) {
    // XXX TEMP static_assert(false, "Argument doesn't have a way to be formatted.");
    assert(false);

    // e.g.
    // void write_custom(fmt_context *f, const my_vector *value) {
    //     fmt_to_writer(f, "x: {}, y: {}", value->x, value->y);
    //     // ...
    // }

    // If you already specialized but still get an error,
    // check if your implementation is in the library namespace.
}

LSTD_END_NAMESPACE
