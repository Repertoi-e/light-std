#pragma once

#include "../io/writer.h"
#include "../memory/stack_dynamic_buffer.h"
#include "debug.h"
#include "parse_context.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

void format_context_write(io::writer *w, const byte *data, s64 count);
void format_context_flush(io::writer *w);

// This writer is kinda specific.
// We have a pointer (_Out_) to a writer that gets the writes with certain formatting behaviour.
//
// We implement _format_context_write_ (the function that io::writer uses) to get format specs into account
// (width, fill char, padding direction etc.) but we provide _write_no_specs(...)_ which directly call Out->write(...).
//
// This object also has functions for writing pointers, integers and floats.
// You can use this without a format string but just directly to write formatted stuff to an output writer.
// @TODO: Separate parse_context from format_context for less confusion.
struct format_context : io::writer {
    io::writer *Out;
    args Args;
    parse_context Parse;

    // null if no specs were parsed
    dynamic_format_specs *Specs = null;

    format_context(io::writer *out, const string &fmtString, const args &args, parse_context::error_handler_t errorHandlerFunc)
        : writer(format_context_write, format_context_flush), Out(out), Args(args), Parse(fmtString, errorHandlerFunc) {}

    using io::writer::write;

    // We need this overload for format_context because otherwise the pointer overload
    // of write_no_specs gets chosen (utf8* gets casted automatically to void*.. sigh!)
    void write(const utf8 *str) { write((const byte *) str, c_string_length(str)); }
    void write(const char8_t *str) { write((const byte *) str, c_string_length(str)); }

    template <types::is_integral T>
    void write(T value) {
        u64 absValue = (u64) value;
        bool negative = sign_bit(value);
        if (negative) absValue = 0 - absValue;

        if (Specs) {
            write_u64(absValue, negative, *Specs);
        } else {
            write_u64(absValue, negative, {});
        }
    }

    template <types::is_floating_point T>
    void write(T value) {
        if (Specs) {
            write_f64((f64) value, *Specs);
        } else {
            write_f64((f64) value, {});
        }
    }

    void write(bool value) {
        if (Specs && Specs->Type) {
            write(value ? 1 : 0);
        } else {
            write(value ? "true" : "false");
        }
    }

    // We checks for specs here, so the non-spec version just calls this one...
    void write(const void *value);

    // Write directly, without looking at formatting specs
    void write_no_specs(const string &str) { Out->write(*((array<byte> *) &str)); }

    // We need this overload for format_context because otherwise the pointer overload
    // of write_no_specs gets chosen (utf8* gets casted automatically to void*.. sigh!)
    void write_no_specs(const utf8 *str) { Out->write((const byte *) str, c_string_length(str)); }
    void write_no_specs(const char8_t *str) { Out->write((const byte *) str, c_string_length(str)); }

    void write_no_specs(const utf8 *str, s64 size) { Out->write((const byte *) str, size); }

    void write_no_specs(utf32 cp) { Out->write(cp); }

    template <types::is_integral T>
    void write_no_specs(T value) {
        u64 absValue = (u64) value;
        bool negative = sign_bit(value);
        if (negative) absValue = 0 - absValue;
        write_u64(absValue, negative, {});
    }

    template <types::is_floating_point T>
    void write_no_specs(T value) {
        write_f64((f64) value, {});
    }

    void write_no_specs(bool value) { write_no_specs(value ? 1 : 0); }

    void write_no_specs(const void *value) {
        auto *old = Specs;
        Specs = null;
        write(value);
        Specs = old;
    }

    // _noSpecs_ means don't take specifiers into account when writing individual arguments in the end
    debug_struct_helper debug_struct(const string &name, bool noSpecs = true) { return debug_struct_helper(this, name, noSpecs); }
    debug_tuple_helper debug_tuple(const string &name, bool noSpecs = true) { return debug_tuple_helper(this, name, noSpecs); }
    debug_list_helper debug_list(bool noSpecs = true) { return debug_list_helper(this, noSpecs); }

    // Returns an argument from index and reports an error if it is out of bounds
    arg get_arg_from_index(s64 index);

    // Checks if fields containing dynamic width/precision (not in-place integers) have been handled and handles them
    // Called by _parse_format_string_ in fmt.cpp
    bool handle_dynamic_specs();

    // The position tells where to point the caret in the format string, so it is clear where exactly the error happened.
    // If left as -1 we calculate using the current Parse.It.
    // We may want to pass a different position if we are in the middle of parsing and the It is not pointing at the right place.
    void on_error(const string &message, s64 position = -1) { Parse.on_error(message, position); }

   private:
    // Writes an integer with given formatting specs
    void write_u64(u64 value, bool negative, format_specs specs);

    // Writes a float with given formatting specs
    void write_f64(f64 value, format_specs specs);
};

namespace internal {
struct format_context_visitor {
    format_context *F;
    bool NoSpecs;

    format_context_visitor(format_context *f, bool noSpecs = false) : F(f), NoSpecs(noSpecs) {}

    void operator()(s32 value) { NoSpecs ? F->write_no_specs(value) : F->write(value); }
    void operator()(u32 value) { NoSpecs ? F->write_no_specs(value) : F->write(value); }
    void operator()(s64 value) { NoSpecs ? F->write_no_specs(value) : F->write(value); }
    void operator()(u64 value) { NoSpecs ? F->write_no_specs(value) : F->write(value); }
    void operator()(bool value) { NoSpecs ? F->write_no_specs(value) : F->write(value); }
    void operator()(f64 value) { NoSpecs ? F->write_no_specs(value) : F->write(value); }
    void operator()(const string &value) { NoSpecs ? F->write_no_specs(value) : F->write(value); }
    void operator()(const void *value) { NoSpecs ? F->write_no_specs(value) : F->write(value); }
    void operator()(const value::custom &custom) { custom.format(F); }

    void operator()(types::unused) { F->on_error("Internal error while formatting"); }
};
}  // namespace internal

}  // namespace fmt

LSTD_END_NAMESPACE