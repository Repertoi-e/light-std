#pragma once

#include "../../storage/stack_dynamic_buffer.h"
#include "../writer.h"
#include "arg.h"
#include "debug.h"
#include "format_float.h"
#include "format_numeric.h"
#include "parse.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

namespace internal {
template <typename F>
void write_padded_helper(format_context *fc, format_specs specs, F f, size_t codePoints) {
    u32 padding = (u32)(specs.Width > codePoints ? specs.Width - codePoints : 0);
    if (specs.Align == alignment::RIGHT) {
        For(range(padding)) fc->write_no_specs(specs.Fill);
        f();
    } else if (specs.Align == alignment::CENTER) {
        u32 leftPadding = padding / 2;
        For(range(leftPadding)) fc->write_no_specs(specs.Fill);
        f();
        For(range(padding - leftPadding)) fc->write_no_specs(specs.Fill);
    } else {
        f();
        For(range(padding)) fc->write_no_specs(specs.Fill);
    }
}
}  // namespace internal

void format_context_write(io::writer *w, const byte *data, size_t count);
void format_context_flush(io::writer *w);

// This writer is kinda specific.
// We have a pointer (_Out_) to a writer that eventually the formatted string
// gets passed to, but we can optimize this with the following:
//
// We don't actually write the string to _Out_ but heavily override it's pointers (and restore them later)
// in order to do a fast flush and prevent copying data.
//
// This works fine if _Out_ is something like a console_writer (where it's internal buffers are only used for caching)
// but if use _format_context_ with _Out_ being a writer where write/flush does specific stuff and such optimization
// actually breaks behaviour, set the _CannotDoFastFlushToOut_ flag to true.
// That way this object actually writes the formatted string to _Out_ before flushing.
//
struct format_context : io::writer, non_copyable, non_movable {
    io::writer *Out;
    args Args;
    internal::arg_map ArgMap;
    parse_context Parse;
    dynamic_format_specs Specs;

    // Whether the current argument getting formatted had any format specs parsed in the format string
    bool HasSpecsForCurrent = false;

    format_context(io::writer *out, string_view fmtString, args args,
                   error_handler_t errorHandlerFunc = default_error_handler)
        : writer(format_context_write, format_context_flush),
          Out(out),
          Args(args),
          Parse(fmtString, errorHandlerFunc) {}

    arg get_arg(size_t index) { return Args.get_arg(index); }
    arg get_arg(string_view name) {
        ArgMap.ensure_initted(Args);
        auto result = ArgMap.find(name);
        if (result.Type == type::NONE) {
            on_error("Argument with this name not found");
        }
        return result;
    }

    using writer::write;

    // Write directly, without taking formatting specs into account.
    void write_no_specs(array_view<byte> data) { Out->write(data); }
    void write_no_specs(const byte *data) { Out->write(data, cstring_strlen(data)); }
    void write_no_specs(const byte *data, size_t count) { Out->write(data, count); }
    void write_no_specs(string str) { Out->write(str); }
    void write_no_specs(char32_t cp) { Out->write(cp); }

    void write_no_specs(s8 value) { write_decimal_no_specs(value); }
    void write_no_specs(s16 value) { write_decimal_no_specs(value); }
    void write_no_specs(s32 value) { write_decimal_no_specs(value); }
    void write_no_specs(s64 value) { write_decimal_no_specs(value); }

    void write_no_specs(u8 value) { write_decimal_no_specs(value); }
    void write_no_specs(u16 value) { write_decimal_no_specs(value); }
    void write_no_specs(u32 value) { write_decimal_no_specs(value); }
    void write_no_specs(u64 value) { write_decimal_no_specs(value); }

    void write_no_specs(bool value) { write_no_specs(value ? 1 : 0); }

    template <typename T>
    enable_if_t<is_floating_point_v<T>> write_no_specs(T value) {
        write_f64((f64) value, {});
    }

    void write_no_specs(const void *value) {
        auto uptr = bit_cast<uptr_t>(value);
        u32 numDigits = COUNT_DIGITS<4>(uptr);

        this->write_no_specs(U'0');
        this->write_no_specs(U'x');

        byte formatBuffer[numeric_info<uptr_t>::digits / 4 + 2];
        auto *p = format_uint_base<4>(formatBuffer, uptr, numDigits);
        this->write_no_specs(p, formatBuffer + numDigits - p);
    }

    template <typename Int>
    struct int_writer {
        using unsigned_t = type_select_t<numeric_info<Int>::digits <= 32, u32, u64>;

        format_context *Context = null;
        unsigned_t AbsValue = 0;
        byte Prefix[4]{};
        size_t PrefixSize = 0;

        int_writer(format_context *context, Int value) : Context(context), AbsValue((unsigned_t) value) {
            if (IS_NEG(value)) {
                AbsValue = 0 - AbsValue;
                Prefix[0] = '-';
                ++PrefixSize;
            } else if (context->Specs.has_flag(flag::PLUS)) {
                Prefix[0] = '+';
                ++PrefixSize;
            } else if (context->Specs.has_flag(flag::SIGN)) {
                Prefix[0] = ' ';
                ++PrefixSize;
            }
        }

        string_view get_prefix() const { return string_view(Prefix, PrefixSize); }

        void on_dec() {
            u32 numDigits = COUNT_DIGITS(AbsValue);
            Context->write_decimal(Context->Specs, get_prefix(),
                                   [&]() {
                                       byte formatBuffer[numeric_info<unsigned_t>::digits10 + 1];
                                       auto *p = format_uint_decimal(formatBuffer, AbsValue, numDigits);
                                       Context->write_no_specs(p, formatBuffer + numDigits - p);
                                   },
                                   numDigits);
        }

        void on_hex() {
            if (Context->Specs.has_flag(flag::HASH)) {
                Prefix[PrefixSize++] = '0';
                Prefix[PrefixSize++] = Context->Specs.Type;
            }
            u32 numDigits = COUNT_DIGITS<4>(AbsValue);
            Context->write_decimal(Context->Specs, get_prefix(),
                                   [&]() {
                                       byte formatBuffer[numeric_info<unsigned_t>::digits / 4 + 2];
                                       auto *p = format_uint_base<4>(formatBuffer, AbsValue, numDigits,
                                                                     is_upper(Context->Specs.Type));
                                       Context->write_no_specs(p, formatBuffer + numDigits - p);
                                   },
                                   numDigits);
        }

        void on_bin() {
            if (Context->Specs.has_flag(flag::HASH)) {
                Prefix[PrefixSize++] = '0';
                Prefix[PrefixSize++] = Context->Specs.Type;
            }
            u32 numDigits = COUNT_DIGITS<1>(AbsValue);
            Context->write_decimal(Context->Specs, get_prefix(),
                                   [&]() {
                                       byte formatBuffer[numeric_info<unsigned_t>::digits + 2];
                                       auto *p = format_uint_base<1>(formatBuffer, AbsValue, numDigits);
                                       Context->write_no_specs(p, formatBuffer + numDigits - p);
                                   },
                                   numDigits);
        }

        void on_oct() {
            u32 numDigits = COUNT_DIGITS<3>(AbsValue);
            if (Context->Specs.has_flag(flag::HASH) && Context->Specs.Precision <= (s32) numDigits) {
                // Octal prefix '0' is counted as a digit,
                // so only add it if precision is not greater than the number of digits.
                Prefix[PrefixSize++] = '0';
            }
            Context->write_decimal(Context->Specs, get_prefix(),
                                   [&]() {
                                       byte formatBuffer[numeric_info<unsigned_t>::digits / 3 + 2];
                                       auto *p = format_uint_base<3>(formatBuffer, AbsValue, numDigits);
                                       Context->write_no_specs(p, formatBuffer + numDigits - p);
                                   },
                                   numDigits);
        }

        void on_num() {
            u32 numDigits = COUNT_DIGITS(AbsValue);
            u32 formattedSize = numDigits + ((numDigits - 1) / 3);

            Context->write_decimal(Context->Specs, get_prefix(),
                                   [&]() {
                                       constexpr auto maxSize = numeric_info<unsigned_t>::digits10 + 1;
                                       byte formatBuffer[maxSize + maxSize / 3];
                                       auto *p = format_uint_decimal(formatBuffer, AbsValue, formattedSize,
                                                                     ".");  // @Locale
                                       Context->write_no_specs(p, formatBuffer + formattedSize - p);
                                   },
                                   numDigits);
        }

        void on_char() {
            auto cp = (char32_t) AbsValue;
            internal::write_padded_helper(Context, Context->Specs, [&]() { Context->write_no_specs(cp); },
                                          get_size_of_cp(cp));
        }

        void on_error() { Context->on_error("Invalid type specifier"); }
    };

    template <typename T>
    enable_if_t<is_integral_v<T>> write(T value) {
        if (HasSpecsForCurrent) {
            auto handler = int_writer<T>(this, value);
            internal::handle_int_type_spec(Specs.Type, &handler);
        } else {
            write_no_specs(value);
        }
    }

    void write(bool value) {
        if (HasSpecsForCurrent && Specs.Type) {
            write(value ? 1 : 0);
        } else {
            write_no_specs(value ? 1 : 0);
        }
    }

    template <typename T>
    enable_if_t<is_floating_point_v<T>> write(T value) {
        if (HasSpecsForCurrent) {
            write_f64((f64) value, Specs);
        } else {
            write_f64((f64) value, {});
        }
    }

    void write(const void *value) {
        if (HasSpecsForCurrent) {
            if (Specs.Type && Specs.Type != 'p') on_error("Invalid type specifier");
        }

        auto uptr = bit_cast<uptr_t>(value);
        u32 numDigits = COUNT_DIGITS<4>(uptr);

        auto f = [&, this]() {
            this->write_no_specs(U'0');
            this->write_no_specs(U'x');
            byte formatBuffer[numeric_info<uptr_t>::digits / 4 + 2];
            auto *p = format_uint_base<4>(formatBuffer, uptr, numDigits);
            this->write_no_specs(p, formatBuffer + numDigits - p);
        };

        if (!HasSpecsForCurrent) {
            f();
            return;
        }

        format_specs specs = Specs;
        if (specs.Align == alignment::DEFAULT) specs.Align = alignment::RIGHT;
        internal::write_padded_helper(this, specs, f, numDigits);
    }

    debug_struct_helper debug_struct(string name) { return debug_struct_helper(this, name); }
    debug_tuple_helper debug_tuple(string name) { return debug_tuple_helper(this, name); }
    debug_list_helper debug_list() { return debug_list_helper(this); }

    void on_error(const byte *message) { Parse.on_error(message); }

   private:
    template <typename>
    friend struct int_writer;

    // Writes an integer in the format
    //   <left-padding><prefix><numeric-padding><digits><right-padding>
    // where <digits> are written by f(this).
    template <typename F>
    void write_decimal(format_specs specs, string_view prefix, F f, u32 numDigits) {
        u32 size = (u32) prefix.Length + numDigits;
        char32_t fill = specs.Fill;
        size_t padding = 0;
        if (specs.Align == alignment::NUMERIC) {
            if (specs.Width > size) {
                padding = specs.Width - size;
                size = specs.Width;
            }
        } else if (specs.Precision > (s32) numDigits) {
            size = (u32) prefix.Length + (u32) specs.Precision;
            padding = (u32) specs.Precision - numDigits;
            fill = '0';
        }

        if (specs.Align == alignment::DEFAULT) specs.Align = alignment::RIGHT;

        internal::write_padded_helper(this, specs,
                                      [&]() {
                                          if (prefix.Length) write_no_specs(prefix);
                                          For(range(padding)) write_no_specs(fill);
                                          f();
                                      },
                                      size);
    }

    // Writes an integer without formatting
    template <typename Int>
    void write_decimal_no_specs(Int value) {
        using abs_t = type_select_t<numeric_info<Int>::digits <= 32, u32, u64>;

        auto absValue = (abs_t)(value);
        bool negative = IS_NEG(value);

        if (negative) absValue = 0 - absValue;

        u32 formattedSize = COUNT_DIGITS(absValue) + (negative ? 1 : 0);
        byte formatBuffer[numeric_info<abs_t>::digits10 + 2];
        byte *p = format_uint_decimal(formatBuffer, absValue, formattedSize);
        if (negative) *--p = '-';
        write_no_specs(formatBuffer, formatBuffer + formattedSize - p);
    }

    struct float_spec_handler {
        format_context *Context;
        byte Type;
        bool Upper = false, Fixed = false, AsPercentage = false;

        float_spec_handler(format_context *context, byte type) : Context(context), Type(type) {}

        void on_general() {
            if (is_upper(Type)) Upper = true;
        }

        void on_exp() {
            if (is_upper(Type)) Upper = true;
        }

        void on_fixed() {
            Fixed = true;
            if (is_upper(Type)) Upper = true;
        }

        void on_percent() {
            Fixed = true;
            AsPercentage = true;
        }

        void on_hex() {
            if (is_upper(Type)) Upper = true;
        }

        void on_error() { Context->on_error("Invalid type specifier"); }
    };

    // Writes a float with given formatting specs (to avoid repetition in the non-spec version)
    void write_f64(f64 value, format_specs specs) {
        float_spec_handler handler(this, specs.Type);
        internal::handle_float_type_spec(specs.Type, &handler);

        char32_t sign = 0;

        ieee754_f64 bits;
        bits.F = value;

        // Check the sign bit instead of value < 0 since the latter is always false for NaN
        if (bits.ieee.S) {
            sign = '-';
            value = -value;
        } else if (specs.has_flag(flag::PLUS)) {
            sign = '+';
        } else if (specs.has_flag(flag::SIGN)) {
            sign = ' ';
        }

        // Handle INF or NAN
        if (bits.ieee.E == 2047) {
            internal::write_padded_helper(this, specs,
                                          [&, this]() {
                                              if (sign) this->write_no_specs(sign);
                                              this->write_no_specs((bits.U & (((1ll) << 52) - 1))
                                                                       ? (handler.Upper ? "NAN" : "nan")
                                                                       : (handler.Upper ? "INF" : "inf"));
                                              if (handler.AsPercentage) this->write_no_specs(U'%');
                                          },
                                          3 + (sign ? 1 : 0) + (handler.AsPercentage ? 1 : 0));
            return;
        }

        if (handler.AsPercentage) value *= 100;

        byte type = specs.Type;
        if (handler.AsPercentage) type = 'f';
        if (!type) type = 'g';

        // @Locale The decimal point written in _internal::format_float_ should be locale-dependent.
        // Also if we decide to add a thousands separator we should do it inside _format_float_
        stack_dynamic_buffer<512> formatBuffer;
        internal::format_float(
            [](void *user, byte *buf, size_t length) {
                auto *fb = (stack_dynamic_buffer<512> *) user;
                fb->ByteLength += length;
                return fb->Data + fb->ByteLength;
            },
            &formatBuffer, formatBuffer.Data, type, value, specs.Precision);

        // Find the decimal point if we are formatting with 'g' and the result is in fixed mode
        // in order to keep only 1 digit after it (python-like formatting).
        if (!specs.Type) {
            auto *p = formatBuffer.begin(), *end = formatBuffer.end();
            while (p < end && is_digit(*p)) ++p;
            if (p < end && to_lower(*p) != 'e') {
                ++p;
                if (*p == '0') ++p;
                while (p != end && *p >= '1' && *p <= '9') ++p;

                char *where = p;
                while (p != end && *p == '0') ++p;

                if (p == end || !is_digit(*p)) {
                    if (p != end) copy_memory(where, p, (size_t)(end - p));
                    formatBuffer.ByteLength -= (size_t)(p - where);
                }
            }
        }

        if (handler.AsPercentage) formatBuffer.append('%');

        if (specs.Align == alignment::NUMERIC) {
            if (sign) {
                write_no_specs(sign);
                sign = 0;
                if (specs.Width) --specs.Width;
            }
            specs.Align = alignment::RIGHT;
        } else if (specs.Align == alignment::DEFAULT) {
            specs.Align = alignment::RIGHT;
        }

        auto formattedSize = formatBuffer.ByteLength;
        if (!specs.Width) {
            formattedSize += sign ? 1 : 0;
        }
        internal::write_padded_helper(this, specs,
                                      [&, this]() { this->write_no_specs(formatBuffer.Data, formatBuffer.ByteLength); },
                                      formattedSize);
    }
};

inline void format_context_write(io::writer *w, const byte *data, size_t count) {
    auto *fc = (format_context *) w;

    if (fc->HasSpecsForCurrent) {
        if (fc->Specs.Type && fc->Specs.Type != 's') fc->on_error("Invalid type specifier");

        size_t length = utf8_strlen(data, count);

        // Adjust size for specified precision
        if (fc->Specs.Precision != -1) {
            assert(fc->Specs.Precision >= 0);
            length = (size_t) fc->Specs.Precision;
            count = get_cp_at_index(data, length, length, true) - data;
        }
        internal::write_padded_helper(fc, fc->Specs, [&]() { fc->write_no_specs(data, count); }, length);
    } else {
        fc->write_no_specs(data, count);
    }
}

inline void format_context_flush(io::writer *w) {
    auto *fc = (format_context *) w;
    fc->Out->flush();
}

namespace internal {

struct format_context_visitor {
    format_context *Context;
    bool NoSpecs;

    format_context_visitor(format_context *context, bool noSpecs = false) : Context(context), NoSpecs(noSpecs) {}

    void operator()(s32 value) { NoSpecs ? Context->write_no_specs(value) : Context->write(value); }
    void operator()(u32 value) { NoSpecs ? Context->write_no_specs(value) : Context->write(value); }
    void operator()(s64 value) { NoSpecs ? Context->write_no_specs(value) : Context->write(value); }
    void operator()(u64 value) { NoSpecs ? Context->write_no_specs(value) : Context->write(value); }
    void operator()(bool value) { NoSpecs ? Context->write_no_specs(value) : Context->write(value); }
    void operator()(f64 value) { NoSpecs ? Context->write_no_specs(value) : Context->write(value); }
    void operator()(array_view<byte> value) { NoSpecs ? Context->write_no_specs(value) : Context->write(value); }
    void operator()(string_view value) { NoSpecs ? Context->write_no_specs(value) : Context->write(value); }
    void operator()(const void *value) { NoSpecs ? Context->write_no_specs(value) : Context->write(value); }

    void operator()(unused) { Context->on_error("Internal error while formatting"); }
    void operator()(arg::handle handle) { Context->on_error("Internal error while formatting a custom argument"); }
};
}  // namespace internal

inline void debug_struct_helper::finish() {
    Context->write_no_specs(Name);
    Context->write_no_specs(" {");

    auto *begin = Fields.begin();
    if (begin != Fields.end()) {
        Context->write_no_specs(" ");
        write_field(begin);
        ++begin;
        while (begin != Fields.end()) {
            Context->write_no_specs(", ");
            write_field(begin);
            ++begin;
        }
    }
    Context->write_no_specs(" }");
}

inline void debug_struct_helper::write_field(debug_struct_field_entry *entry) {
    Context->write_no_specs(entry->Name);
    Context->write_no_specs(": ");
    visit_fmt_arg(internal::format_context_visitor(Context, true), entry->Arg);
}

inline void debug_tuple_helper::finish() {
    Context->write_no_specs(Name);
    Context->write_no_specs("(");

    auto *begin = Fields.begin();
    if (begin != Fields.end()) {
        visit_fmt_arg(internal::format_context_visitor(Context, true), *begin);
        ++begin;
        while (begin != Fields.end()) {
            Context->write_no_specs(", ");
            visit_fmt_arg(internal::format_context_visitor(Context, true), *begin);
            ++begin;
        }
    }
    Context->write_no_specs(")");
}

inline void fmt::debug_list_helper::finish() {
    Context->write_no_specs("[");

    auto *begin = Fields.begin();
    if (begin != Fields.end()) {
        visit_fmt_arg(internal::format_context_visitor(Context, true), *begin);
        ++begin;
        while (begin != Fields.end()) {
            Context->write_no_specs(", ");
            visit_fmt_arg(internal::format_context_visitor(Context, true), *begin);
            ++begin;
        }
    }
    Context->write_no_specs("]");
}

}  // namespace fmt

LSTD_END_NAMESPACE