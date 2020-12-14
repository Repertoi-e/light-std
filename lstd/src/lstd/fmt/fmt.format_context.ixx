module;

#include "../types/numeric_info.h"
#include "../io.h"
#include "format_float.inl"
#include "parse_error_handler.h"

export module fmt.format_context;

import fmt.arg;
import fmt.specs;
import fmt.parse_context;

export {
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
        fmt_args Args;
        parse_context Parse;

        // null if no specs were parsed.
        // When writing a custom formatter use this for checking specifiers.
        // e.g.
        //     if (f->Specs && f->Specs...) { ... }
        dynamic_format_specs *Specs = null;

        format_context(writer *out, const string &fmtString, const fmt_args &args, parse_error_handler_t errorHandlerFunc)
            : Out(out), Args(args), Parse(fmtString, errorHandlerFunc) {}

        void write(const byte *data, s64 count) override;
        void flush() override { Out->flush(); }
    };

    // The position tells where to point the caret in the format string, so it is clear where exactly the error happened.
    // If left as -1 we calculate using the current Parse.It.
    // We may want to pass a different position if we are in the middle of parsing and the It is not pointing at the right place.
    inline void on_error(format_context * f, const string &message, s64 position = -1) { on_error(&f->Parse, message, position); }

    // Writes an integer with given formatting specs
    void write_u64(format_context * f, u64 value, bool negative, format_specs specs);

    // Writes a float with given formatting specs
    void write_f64(format_context * f, f64 value, format_specs specs);

    // We need this overload for format_context because otherwise the pointer overload
    // of write_no_specs gets chosen (utf8* gets casted automatically to void*.. sigh!)
    inline void write(format_context * f, const utf8 *str) { f->write((const byte *) str, c_string_length(str)); }
    inline void write(format_context * f, const char8_t *str) { f->write((const byte *) str, c_string_length(str)); }

    template <types::is_integral T>
    void write(format_context * f, T value) {
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
    void write(format_context * f, T value) {
        if (f->Specs) {
            write_f64(f, (f64) value, *f->Specs);
        } else {
            write_f64(f, (f64) value, {});
        }
    }

    void write(format_context * f, bool value);

    // We check for specs here, so the non-spec version just calls this one...
    void write(format_context * f, const void *value);

    // Write directly, without looking at formatting specs
    inline void write_no_specs(format_context * f, const string &str) { write(f->Out, *((array<byte> *) &str)); }

    // We need this overload for format_context because otherwise the pointer overload
    // of write_no_specs gets chosen (utf8* gets casted automatically to void*.. sigh!)
    inline void write_no_specs(format_context * f, const utf8 *str) { write(f->Out, (const byte *) str, c_string_length(str)); }
    inline void write_no_specs(format_context * f, const char8_t *str) { write(f->Out, (const byte *) str, c_string_length(str)); }

    inline void write_no_specs(format_context * f, const utf8 *str, s64 size) { write(f->Out, (const byte *) str, size); }

    inline void write_no_specs(format_context * f, utf32 cp) { write(f->Out, cp); }

    template <types::is_integral T>
    void write_no_specs(format_context * f, T value) {
        u64 absValue = (u64) value;
        bool negative = sign_bit(value);
        if (negative) absValue = 0 - absValue;
        write_u64(f, absValue, negative, {});
    }

    template <types::is_floating_point T>
    void write_no_specs(format_context * f, T value) {
        write_f64(f, (f64) value, {});
    }

    inline void write_no_specs(format_context * f, bool value) { write_no_specs(f, value ? 1 : 0); }

    inline void write_no_specs(format_context * f, const void *value) {
        auto *old = f->Specs;
        f->Specs = null;
        write(f, value);
        f->Specs = old;
    }

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
        void operator()(const value::custom &custom) { custom.FormatFunc(F, custom.Data); }

        void operator()(types::unused) {
            on_error(F, "Internal error while formatting");
            assert(false);
        }
    };
    }  // namespace internal

    struct format_struct_helper {
        struct field_entry {
            string Name;
            fmt_arg Arg;
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
            append(Fields, {name, fmt_make_arg(value)});
            return this;
        }

        void finish() {
            write_no_specs(F, Name);
            write_no_specs(F, " {");

            auto *begin = Fields.begin();
            if (begin != Fields.end()) {
                write_no_specs(F, " ");
                write_field(begin);
                ++begin;
                while (begin != Fields.end()) {
                    write_no_specs(F, ", ");
                    write_field(begin);
                    ++begin;
                }
            }
            write_no_specs(F, " }");
        }

       private:
        void write_field(field_entry *entry) {
            write_no_specs(F, entry->Name);
            write_no_specs(F, ": ");
            fmt_visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), entry->Arg);
        }
    };

    struct format_tuple_helper {
        format_context *F;
        string Name;
        array<fmt_arg> Fields;
        bool NoSpecs;  // Write the result without taking into account specs for individual arguments

        format_tuple_helper(format_context *f, const string &name, bool noSpecs) : F(f), Name(name), NoSpecs(noSpecs) {}

        // I know we are against hidden freeing but having this destructor is actually really fine.
        // Things would be a whole more ugly and complicated without it.
        ~format_tuple_helper() { free(Fields); }

        template <typename T>
        format_tuple_helper *field(const T &value) {
            append(Fields, fmt_make_arg(value));
            return this;
        }

        void finish() {
            write_no_specs(F, Name);
            write_no_specs(F, "(");

            auto *begin = Fields.begin();
            if (begin != Fields.end()) {
                fmt_visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), *begin);
                ++begin;
                while (begin != Fields.end()) {
                    write_no_specs(F, ", ");
                    fmt_visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), *begin);
                    ++begin;
                }
            }
            write_no_specs(F, ")");
        }
    };

    struct format_list_helper {
        format_context *F;
        array<fmt_arg> Fields;
        bool NoSpecs;  // Write the result without taking into account specs for individual arguments

        format_list_helper(format_context *f, bool noSpecs) : F(f), NoSpecs(noSpecs) {}

        // I know we are against hidden freeing but having this destructor is actually really fine.
        // Things would be a whole more ugly and complicated without it.
        ~format_list_helper() { free(Fields); }

        template <typename T>
        format_list_helper *entries(const array_view<T> &values) {
            For(values) append(Fields, fmt_make_arg(it));
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

        void finish() {
            write_no_specs(F, "[");

            auto *begin = Fields.begin();
            if (begin != Fields.end()) {
                fmt_visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), *begin);
                ++begin;
                while (begin != Fields.end()) {
                    write_no_specs(F, ", ");
                    fmt_visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), *begin);
                    ++begin;
                }
            }
            write_no_specs(F, "]");
        }
    };

    // _noSpecs_ means don't take specifiers into account when writing individual arguments in the end
    // These return an object which collects elements and then outputs them in the following way:
    // struct:    *name* { field1: value, field2: value, ... }
    // tuple:     *name*(element1, element2, ...)
    // list:      [element1, element2, ...]
    format_struct_helper format_struct(format_context * f, const string &name, bool noSpecs = true) { return format_struct_helper(f, name, noSpecs); }
    format_tuple_helper format_tuple(format_context * f, const string &name, bool noSpecs = true) { return format_tuple_helper(f, name, noSpecs); }
    format_list_helper format_list(format_context * f, bool noSpecs = true) { return format_list_helper(f, noSpecs); }
}

file_scope utf8 DIGITS[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

template <typename UInt>
file_scope utf8 *format_uint_decimal(utf8 *buffer, UInt value, s64 formattedSize, const string &thousandsSep = "") {
    u32 digitIndex = 0;

    buffer += formattedSize;
    while (value >= 100) {
        u32 index = (u32)(value % 100) * 2;
        value /= 100;
        *--buffer = DIGITS[index + 1];
        if (++digitIndex % 3 == 0) {
            buffer -= thousandsSep.Count;
            copy_memory(buffer, thousandsSep.Data, thousandsSep.Count);
        }
        *--buffer = DIGITS[index];
        if (++digitIndex % 3 == 0) {
            buffer -= thousandsSep.Count;
            copy_memory(buffer, thousandsSep.Data, thousandsSep.Count);
        }
    }

    if (value < 10) {
        *--buffer = (utf8)('0' + value);
        return buffer;
    }

    u32 index = (u32) value * 2;
    *--buffer = DIGITS[index + 1];
    if (++digitIndex % 3 == 0) {
        buffer -= thousandsSep.Count;
        copy_memory(buffer, thousandsSep.Data, thousandsSep.Count);
    }
    *--buffer = DIGITS[index];

    return buffer;
}

template <u32 BASE_BITS, typename UInt>
file_scope utf8 *format_uint_base(utf8 *buffer, UInt value, s64 formattedSize, bool upper = false) {
    buffer += formattedSize;
    do {
        const utf8 *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
        u32 digit = (value & ((1 << BASE_BITS) - 1));
        *--buffer = (utf8)(BASE_BITS < 4 ? (utf8)('0' + digit) : digits[digit]);
    } while ((value >>= BASE_BITS) != 0);
    return buffer;
}

// Writes pad code points and the actual contents with f(),
// _fSize_ needs to be the size of the output from _f_ in code points (in order to calculate padding properly)
template <typename F>
file_scope void write_padded_helper(format_context *f, const format_specs &specs, F &&func, s64 fSize) {
    u32 padding = (u32)(specs.Width > fSize ? specs.Width - fSize : 0);
    if (specs.Align == alignment::RIGHT) {
        For(range(padding)) write_no_specs(f, specs.Fill);
        func();
    } else if (specs.Align == alignment::CENTER) {
        u32 leftPadding = padding / 2;
        For(range(leftPadding)) write_no_specs(f, specs.Fill);
        func();
        For(range(padding - leftPadding)) write_no_specs(f, specs.Fill);
    } else {
        func();
        For(range(padding)) write_no_specs(f, specs.Fill);
    }
}

void write_helper(format_context *f, const byte *data, s64 size) {
    if (!f->Specs) {
        write_no_specs(f, (const utf8 *) data, size);
        return;
    }

    if (f->Specs->Type) {
        if (f->Specs->Type == 'p') {
            write(f, (const void *) data);
            return;
        }
        if (f->Specs->Type != 's') {
            on_error(f, "Invalid type specifier for a string", f->Parse.It.Data - f->Parse.FormatString.Data - 1);
            return;
        }
    }

    // 'p' wasn't specified, not treating as formatting a pointer
    s64 length = utf8_length((const utf8 *) data, size);

    // Adjust size for specified precision
    if (f->Specs->Precision != -1) {
        assert(f->Specs->Precision >= 0);
        length = f->Specs->Precision;
        size = get_cp_at_index((const utf8 *) data, length, length, true) - (const utf8 *) data;
    }
    write_padded_helper(
        f, *f->Specs, [&]() { write_no_specs(f, (const utf8 *) data, size); }, length);
}

void format_context::write(const byte *data, s64 count) { write_helper(this, data, count); }

// @Threadsafety ???
file_scope utf8 U64_FORMAT_BUFFER[numeric_info<u64>::digits10 + 1];

void write(format_context *f, bool value) {
    if (f->Specs && f->Specs->Type) {
        write(f, value ? 1 : 0);
    } else {
        write(f, value ? "true" : "false");
    }
}

void write(format_context *f, const void *value) {
    if (f->Specs && f->Specs->Type && f->Specs->Type != 'p') {
        on_error(f, "Invalid type specifier for a pointer", f->Parse.It.Data - f->Parse.FormatString.Data - 1);
        return;
    }

    auto uptr = bit_cast<u64>(value);
    u32 numDigits = count_digits<4>(uptr);

    auto func = [&, f]() {
        write_no_specs(f, U'0');
        write_no_specs(f, U'x');

        utf8 formatBuffer[numeric_info<u64>::digits / 4 + 2];
        auto *p = format_uint_base<4>(formatBuffer, uptr, numDigits);
        write_no_specs(f, p, formatBuffer + numDigits - p);
    };

    if (!f->Specs) {
        func();
        return;
    }

    format_specs specs = *f->Specs;
    if (specs.Align == alignment::NONE) specs.Align = alignment::RIGHT;
    write_padded_helper(f, specs, func, numDigits + 2);
}

void write_u64(format_context *f, u64 value, bool negative, format_specs specs) {
    utf8 type = specs.Type;
    if (!type) type = 'd';

    s64 numDigits;
    if (type == 'd' || type == 'n') {
        numDigits = count_digits(value);
    } else if (to_lower(type) == 'b') {
        numDigits = count_digits<1>(value);
    } else if (type == 'o') {
        numDigits = count_digits<3>(value);
    } else if (to_lower(type) == 'x') {
        numDigits = count_digits<4>(value);
    } else if (type == 'c') {
        if (specs.Align == alignment::NUMERIC || specs.Sign != fmt_sign::NONE || specs.Hash) {
            on_error(f, "Invalid format specifier(s) for code point - code points can't have numeric alignment, signs or #", f->Parse.It.Data - f->Parse.FormatString.Data);
            return;
        }
        auto cp = (utf32) value;
        write_padded_helper(
            f, specs, [&]() { write_no_specs(f, cp); }, get_size_of_cp(cp));
        return;
    } else {
        on_error(f, "Invalid type specifier for an integer", f->Parse.It.Data - f->Parse.FormatString.Data - 1);
        return;
    }

    utf8 prefixBuffer[4];
    utf8 *prefixPointer = prefixBuffer;

    if (negative) {
        *prefixPointer++ = '-';
    } else if (specs.Sign == fmt_sign::PLUS) {
        *prefixPointer++ = '+';
    } else if (specs.Sign == fmt_sign::SPACE) {
        *prefixPointer++ = ' ';
    }

    if ((to_lower(type) == 'x' || to_lower(type) == 'b') && specs.Hash) {
        *prefixPointer++ = '0';
        *prefixPointer++ = type;
    }

    // Octal prefix '0' is counted as a digit,
    // so only add it if precision is not greater than the number of digits.
    if (type == 'o' && specs.Hash) {
        if (specs.Precision == -1 || specs.Precision > numDigits) *prefixPointer++ = '0';
    }

    auto prefix = string(prefixBuffer, prefixPointer - prefixBuffer);

    s64 formattedSize = prefix.Length + numDigits;
    s64 padding = 0;
    if (specs.Align == alignment::NUMERIC) {
        if (specs.Width > formattedSize) {
            padding = specs.Width - formattedSize;
            formattedSize = specs.Width;
        }
    } else if (specs.Precision > (s32) numDigits) {
        formattedSize = (u32) prefix.Length + (u32) specs.Precision;
        padding = (u32) specs.Precision - numDigits;
        specs.Fill = '0';
    }
    if (specs.Align == alignment::NONE) specs.Align = alignment::RIGHT;

    type = (utf8) to_lower(type);
    if (type == 'd') {
        write_padded_helper(
            f, specs,
            [&]() {
                if (prefix.Length) write_no_specs(f, prefix);
                For(range(padding)) write_no_specs(f, specs.Fill);
                auto *p = format_uint_decimal(U64_FORMAT_BUFFER, value, numDigits);
                write_no_specs(f, p, U64_FORMAT_BUFFER + numDigits - p);
            },
            formattedSize);
    } else if (type == 'b') {
        write_padded_helper(
            f, specs,
            [&]() {
                if (prefix.Length) write_no_specs(f, prefix);
                For(range(padding)) write_no_specs(f, specs.Fill);
                auto *p = format_uint_base<1>(U64_FORMAT_BUFFER, value, numDigits);
                write_no_specs(f, p, U64_FORMAT_BUFFER + numDigits - p);
            },
            formattedSize);
    } else if (type == 'o') {
        write_padded_helper(
            f, specs,
            [&]() {
                if (prefix.Length) write_no_specs(f, prefix);
                For(range(padding)) write_no_specs(f, specs.Fill);
                auto *p = format_uint_base<3>(U64_FORMAT_BUFFER, value, numDigits);
                write_no_specs(f, p, U64_FORMAT_BUFFER + numDigits - p);
            },
            formattedSize);
    } else if (type == 'x') {
        write_padded_helper(
            f, specs,
            [&]() {
                if (prefix.Length) write_no_specs(f, prefix);
                For(range(padding)) write_no_specs(f, specs.Fill);
                auto *p = format_uint_base<4>(U64_FORMAT_BUFFER, value, numDigits, is_upper(specs.Type));
                write_no_specs(f, p, U64_FORMAT_BUFFER + numDigits - p);
            },
            formattedSize);
    } else if (type == 'n') {
        formattedSize += ((numDigits - 1) / 3);
        write_padded_helper(
            f, specs,
            [&]() {
                if (prefix.Length) write_no_specs(f, prefix);
                For(range(padding)) write_no_specs(f, specs.Fill);
                auto *p = format_uint_decimal(U64_FORMAT_BUFFER, value, formattedSize, "," /*@Locale*/);
                write_no_specs(f, p, U64_FORMAT_BUFFER + formattedSize - p);
            },
            formattedSize);
    } else {
        assert(false && "Invalid type");  // sanity
    }
}

// Writes a float with given formatting specs
void write_f64(format_context *f, f64 value, format_specs specs) {
    utf8 type = specs.Type;
    if (type) {
        utf8 lower = (utf8) to_lower(type);
        if (lower != 'g' && lower != 'e' && lower != '%' && lower != 'f' && lower != 'a') {
            on_error(f, "Invalid type specifier for a float", f->Parse.It.Data - f->Parse.FormatString.Data - 1);
            return;
        }
    } else {
        type = 'g';
    }

    bool percentage = specs.Type == '%';

    utf32 sign = 0;

    ieee754_f64 bits;
    bits.F = value;

    // Check the sign bit instead of value < 0 since the latter is always false for NaN
    if (bits.ieee.S) {
        sign = '-';
        value = -value;
    } else if (specs.Sign == fmt_sign::PLUS) {
        sign = '+';
    } else if (specs.Sign == fmt_sign::SPACE) {
        sign = ' ';
    }

    // Handle INF or NAN
    if (bits.ieee.E == 2047) {
        write_padded_helper(
            f, specs,
            [&]() {
                if (sign) write_no_specs(f, sign);
                write_no_specs(f, (bits.W & ((1ll << 52) - 1)) ? (is_upper(specs.Type) ? "NAN" : "nan") : (is_upper(specs.Type) ? "INF" : "inf"));
                if (percentage) write_no_specs(f, U'%');
            },
            3 + (sign ? 1 : 0) + (percentage ? 1 : 0));
        return;
    }

    if (percentage) {
        value *= 100;
        type = 'f';
    }

    // @Locale The decimal point written in _internal::format_float_ should be locale-dependent.
    // Also if we decide to add a thousands separator we should do it inside _format_float_
    stack_dynamic_buffer<512> formatBuffer;
    defer(free(formatBuffer));

    format_float(
        [](void *user, utf8 *buf, s64 length) {
            auto *fb = (stack_dynamic_buffer<512> *) user;
            fb->Count += length;
            return (utf8 *) fb->Data + fb->Count;
        },
        &formatBuffer, (utf8 *) formatBuffer.Data, type, value, specs.Precision);

    // Note: We set _type_ to 'g' if it's zero, but here we check specs.Type (which we didn't modify)
    // This is because '0' is similar to 'g', except that it prints at least one digit after the decimal point,
    // which we do here (python-like formatting)
    if (!specs.Type) {
        auto *p = formatBuffer.begin(), *end = formatBuffer.end();
        while (p < end && is_digit(*p)) ++p;
        if (p < end && to_lower(*p) != 'e') {
            ++p;
            if (*p == '0') ++p;
            while (p != end && *p >= '1' && *p <= '9') ++p;

            byte *where = p;
            while (p != end && *p == '0') ++p;

            if (p == end || !is_digit(*p)) {
                if (p != end) copy_memory(where, p, (s64)(end - p));
                formatBuffer.Count -= (s64)(p - where);
            }
        } else if (p == end) {
            // There was no dot at all
            append_pointer_and_size(formatBuffer, (byte *) ".0", 2);
        }
    }

    if (percentage) append(formatBuffer, '%');

    if (specs.Align == alignment::NUMERIC) {
        if (sign) {
            write_no_specs(f, sign);
            sign = 0;
            if (specs.Width) --specs.Width;
        }
        specs.Align = alignment::RIGHT;
    } else if (specs.Align == alignment::NONE) {
        specs.Align = alignment::RIGHT;
    }

    auto formattedSize = formatBuffer.Count + (sign ? 1 : 0);
    write_padded_helper(
        f, specs,
        [&]() {
            if (sign) write_no_specs(f, sign);
            write_no_specs(f, (utf8 *) formatBuffer.Data, formatBuffer.Count);
        },
        formattedSize);
}
