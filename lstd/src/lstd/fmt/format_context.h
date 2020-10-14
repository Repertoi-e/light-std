#pragma once

#include "../io/writer.h"
#include "../memory/stack_dynamic_buffer.h"
#include "debug.h"
#include "parse_context.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

// This writer is kinda specific.
// We have a pointer (_Out_) to a writer that gets the writes with certain formatting behaviour.
//
// We implement _format_context_write_ (the function that writer uses) to get format specs into account
// (width, fill char, padding direction etc.) but we provide _write_no_specs(...)_ which directly call Out->write(...).
//
// This object also has functions for writing pointers, integers and floats.
// You can use this without a format string but just directly to write formatted stuff to an output writer.
// @TODO: Separate parse_context from format_context for less confusion.
struct format_context : writer {
    writer *Out;
    args Args;
    parse_context Parse;

    // null if no specs were parsed
    dynamic_format_specs *Specs = null;

    format_context(writer *out, const string &fmtString, const args &args, parse_error_handler_t errorHandlerFunc)
        : Out(out), Args(args), Parse(fmtString, errorHandlerFunc) {}

    void write(const byte *data, s64 count) override;
    void flush() override { Out->flush(); }
};

// The position tells where to point the caret in the format string, so it is clear where exactly the error happened.
// If left as -1 we calculate using the current Parse.It.
// We may want to pass a different position if we are in the middle of parsing and the It is not pointing at the right place.
inline void on_error(format_context *f, const string &message, s64 position = -1) { on_error(&f->Parse, message, position); }

// Writes an integer with given formatting specs
void write_u64(format_context *f, u64 value, bool negative, format_specs specs);

// Writes a float with given formatting specs
void write_f64(format_context *f, f64 value, format_specs specs);

// We need this overload for format_context because otherwise the pointer overload
// of write_no_specs gets chosen (utf8* gets casted automatically to void*.. sigh!)
inline void write(format_context *f, const utf8 *str) { f->write((const byte *) str, c_string_length(str)); }
inline void write(format_context *f, const char8_t *str) { f->write((const byte *) str, c_string_length(str)); }

template <types::is_integral T>
void write(format_context *f, T value) {
    u64 absValue = (u64) value;
    bool negative = sign_bit(value);
    if (negative) absValue = 0 - absValue;

    if (f->Specs) {
        write_u64(f, absValue, negative, *f->Specs);
    } else {
        write_u64(f, absValue, negative, {});
    }
}

template <types::is_floating_point T>
void write(format_context *f, T value) {
    if (f->Specs) {
        write_f64(f, (f64) value, *f->Specs);
    } else {
        write_f64(f, (f64) value, {});
    }
}

void write(format_context *f, bool value);

// We check for specs here, so the non-spec version just calls this one...
void write(format_context *f, const void *value);

// Write directly, without looking at formatting specs
inline void write_no_specs(format_context *f, const string &str) { write(f->Out, *((array<byte> *) &str)); }

// We need this overload for format_context because otherwise the pointer overload
// of write_no_specs gets chosen (utf8* gets casted automatically to void*.. sigh!)
inline void write_no_specs(format_context *f, const utf8 *str) { write(f->Out, (const byte *) str, c_string_length(str)); }
inline void write_no_specs(format_context *f, const char8_t *str) { write(f->Out, (const byte *) str, c_string_length(str)); }

inline void write_no_specs(format_context *f, const utf8 *str, s64 size) { write(f->Out, (const byte *) str, size); }

inline void write_no_specs(format_context *f, utf32 cp) { write(f->Out, cp); }

template <types::is_integral T>
void write_no_specs(format_context *f, T value) {
    u64 absValue = (u64) value;
    bool negative = sign_bit(value);
    if (negative) absValue = 0 - absValue;
    write_u64(f, absValue, negative, {});
}

template <types::is_floating_point T>
void write_no_specs(format_context *f, T value) {
    write_f64(f, (f64) value, {});
}

inline void write_no_specs(format_context *f, bool value) { write_no_specs(f, value ? 1 : 0); }

inline void write_no_specs(format_context *f, const void *value) {
    auto *old = f->Specs;
    f->Specs = null;
    write(f, value);
    f->Specs = old;
}

// _noSpecs_ means don't take specifiers into account when writing individual arguments in the end
// These return an object which collects elements and then outputs them in the following way:
// struct:    *name* { field1: value, field2: value, ... }
// tuple:     *name*(element1, element2, ...)
// list:      [element1, element2, ...]
inline format_struct_helper format_struct(format_context *f, const string &name, bool noSpecs = true) { return format_struct_helper(f, name, noSpecs); }
inline format_tuple_helper format_tuple(format_context *f, const string &name, bool noSpecs = true) { return format_tuple_helper(f, name, noSpecs); }
inline format_list_helper format_list(format_context *f, bool noSpecs = true) { return format_list_helper(f, noSpecs); }

namespace internal {
struct format_context_visitor {
    format_context *F;
    bool NoSpecs;

    format_context_visitor(format_context *f, bool noSpecs = false) : F(f), NoSpecs(noSpecs) {}

    void operator()(s32 value) { NoSpecs ? write_no_specs(F, value) : write(F, value); }
    void operator()(u32 value) { NoSpecs ? write_no_specs(F, value) : write(F, value); }
    void operator()(s64 value) { NoSpecs ? write_no_specs(F, value) : write(F, value); }
    void operator()(u64 value) { NoSpecs ? write_no_specs(F, value) : write(F, value); }
    void operator()(bool value) { NoSpecs ? write_no_specs(F, value) : write(F, value); }
    void operator()(f64 value) { NoSpecs ? write_no_specs(F, value) : write(F, value); }
    void operator()(const string &value) { NoSpecs ? write_no_specs(F, value) : write(F, value); }
    void operator()(const void *value) { NoSpecs ? write_no_specs(F, value) : write(F, value); }
    void operator()(const value::custom &custom) { custom.format(F); }

    void operator()(types::unused) { on_error(F, "Internal error while formatting"); }
};
}  // namespace internal

}  // namespace fmt

LSTD_END_NAMESPACE