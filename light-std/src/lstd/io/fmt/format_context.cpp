#include "format_context.h"
#include "format_float.inl"

LSTD_BEGIN_NAMESPACE

namespace fmt {

static byte DIGITS[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

template <typename UInt>
static byte *format_uint_decimal(byte *buffer, UInt value, size_t formattedSize, string_view thousandsSep = "") {
    u32 digitIndex = 0;

    buffer += formattedSize;
    while (value >= 100) {
        u32 index = (u32)(value % 100) * 2;
        value /= 100;
        *--buffer = (byte) DIGITS[index + 1];
        if (++digitIndex % 3 == 0) {
            buffer -= thousandsSep.ByteLength;
            copy_memory(buffer, thousandsSep.Data, thousandsSep.ByteLength);
        }
        *--buffer = (byte) DIGITS[index];
        if (++digitIndex % 3 == 0) {
            buffer -= thousandsSep.ByteLength;
            copy_memory(buffer, thousandsSep.Data, thousandsSep.ByteLength);
        }
    }

    if (value < 10) {
        *--buffer = (byte)('0' + value);
        return buffer;
    }

    u32 index = (u32) value * 2;
    *--buffer = (byte) DIGITS[index + 1];
    if (++digitIndex % 3 == 0) {
        buffer -= thousandsSep.ByteLength;
        copy_memory(buffer, thousandsSep.Data, thousandsSep.ByteLength);
    }
    *--buffer = (byte) DIGITS[index];

    return buffer;
}

template <u32 BASE_BITS, typename UInt>
static byte *format_uint_base(byte *buffer, UInt value, size_t formattedSize, bool upper = false) {
    buffer += formattedSize;
    do {
        const byte *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
        u32 digit = (value & ((1 << BASE_BITS) - 1));
        *--buffer = (byte)(BASE_BITS < 4 ? (byte)('0' + digit) : digits[digit]);
    } while ((value >>= BASE_BITS) != 0);
    return buffer;
}

// Writes pad code points and the actual contents with f(),
// _fSize_ needs to be the size of the output from _f_ in code points (in order to calculate padding properly)
template <typename F>
static void write_padded_helper(format_context *f, const format_specs &specs, F &&func, size_t fSize) {
    u32 padding = (u32)(specs.Width > fSize ? specs.Width - fSize : 0);
    if (specs.Align == alignment::RIGHT) {
        For(range(padding)) f->write_no_specs(specs.Fill);
        func();
    } else if (specs.Align == alignment::CENTER) {
        u32 leftPadding = padding / 2;
        For(range(leftPadding)) f->write_no_specs(specs.Fill);
        func();
        For(range(padding - leftPadding)) f->write_no_specs(specs.Fill);
    } else {
        func();
        For(range(padding)) f->write_no_specs(specs.Fill);
    }
}

void format_context_write(io::writer *w, const byte *data, size_t count) {
    auto *f = (format_context *) w;

    if (!f->Specs) {
        f->write_no_specs(data, count);
        return;
    }

    if (f->Specs->Type) {
        if (f->Specs->Type == 'p') {
            f->write((void *) data);
            return;
        }
        if (f->Specs->Type != 's') {
            f->on_error("Invalid type specifier");
            return;
        }
    }

    // 'p' wasn't specified, not treating as formatting a pointer
    size_t length = utf8_strlen(data, count);

    // Adjust size for specified precision
    if (f->Specs->Precision != -1) {
        assert(f->Specs->Precision >= 0);
        length = (size_t) f->Specs->Precision;
        count = get_cp_at_index(data, length, length, true) - data;
    }
    write_padded_helper(f, *f->Specs, [&]() { f->write_no_specs(data, count); }, length);
}

void format_context_flush(io::writer *w) {
    auto *f = (format_context *) w;
    f->Out->flush();
}

static byte U64_FORMAT_BUFFER[numeric_info<u64>::digits10 + 1];

void format_context::write(const void *value) {
    if (Specs && Specs->Type && Specs->Type != 'p') {
        on_error("Invalid type specifier");
        return;
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

    if (!Specs) {
        f();
        return;
    }

    format_specs specs = *Specs;
    if (specs.Align == alignment::DEFAULT) specs.Align = alignment::RIGHT;
    write_padded_helper(this, specs, f, numDigits + 2);
}

void format_context::write_u64(u64 value, bool negative, format_specs specs) {
    byte type = specs.Type;
    if (!type) type = 'd';

    size_t numDigits;
    if (type == 'd' || type == 'n') {
        numDigits = COUNT_DIGITS(value);
    } else if (to_lower(type) == 'b') {
        numDigits = COUNT_DIGITS<1>(value);
    } else if (type == 'o') {
        numDigits = COUNT_DIGITS<3>(value);
    } else if (to_lower(type) == 'x') {
        numDigits = COUNT_DIGITS<4>(value);
    } else if (type == 'c') {
        if (specs.Align == alignment::NUMERIC || specs.has_flag(flag::SIGN) || specs.has_flag(flag::PLUS) ||
            specs.has_flag(flag::MINUS) || specs.has_flag(flag::HASH)) {
            on_error("Invalid format specifier for code point");
            return;
        }
        auto cp = (char32_t) value;
        write_padded_helper(this, specs, [&]() { this->write_no_specs(cp); }, get_size_of_cp(cp));
        return;
    } else {
        on_error("Invalid type specifier");
        return;
    }

    byte prefixBuffer[4];
    byte *prefixPointer = prefixBuffer;

    if (negative) {
        *prefixPointer++ = '-';
    } else if (specs.has_flag(flag::PLUS)) {
        *prefixPointer++ = '+';
    } else if (specs.has_flag(flag::SIGN)) {
        *prefixPointer++ = ' ';
    }

    if ((to_lower(type) == 'x' || to_lower(type) == 'b') && specs.has_flag(flag::HASH)) {
        *prefixPointer++ = '0';
        *prefixPointer++ = type;
    }

    // Octal prefix '0' is counted as a digit,
    // so only add it if precision is not greater than the number of digits.
    // Note: Here if _specs.Precision_ is -1 (i.e. not specified)
    // the cast to size_t will overflow it to a really high value
    if (type == 'o' && specs.has_flag(flag::HASH) && (size_t) specs.Precision > numDigits) {
        *prefixPointer++ = '0';
    }

    auto prefix = string_view(prefixBuffer, prefixPointer - prefixBuffer);

    size_t formattedSize = prefix.Length + numDigits;
    size_t padding = 0;
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
    if (specs.Align == alignment::DEFAULT) specs.Align = alignment::RIGHT;

    type = (byte) to_lower(type);
    if (type == 'd') {
        write_padded_helper(this, specs,
                            [&]() {
                                if (prefix.Length) this->write_no_specs(prefix);
                                For(range(padding)) this->write_no_specs(specs.Fill);
                                auto *p = format_uint_decimal(U64_FORMAT_BUFFER, value, numDigits);
                                this->write_no_specs(p, U64_FORMAT_BUFFER + numDigits - p);
                            },
                            formattedSize);
    } else if (type == 'b') {
        write_padded_helper(this, specs,
                            [&]() {
                                if (prefix.Length) this->write_no_specs(prefix);
                                For(range(padding)) this->write_no_specs(specs.Fill);
                                auto *p = format_uint_base<1>(U64_FORMAT_BUFFER, value, numDigits);
                                this->write_no_specs(p, U64_FORMAT_BUFFER + numDigits - p);
                            },
                            formattedSize);
    } else if (type == 'o') {
        write_padded_helper(this, specs,
                            [&]() {
                                if (prefix.Length) this->write_no_specs(prefix);
                                For(range(padding)) this->write_no_specs(specs.Fill);
                                auto *p = format_uint_base<3>(U64_FORMAT_BUFFER, value, numDigits);
                                this->write_no_specs(p, U64_FORMAT_BUFFER + numDigits - p);
                            },
                            formattedSize);
    } else if (type == 'x') {
        write_padded_helper(this, specs,
                            [&]() {
                                if (prefix.Length) this->write_no_specs(prefix);
                                For(range(padding)) this->write_no_specs(specs.Fill);
                                auto *p =
                                    format_uint_base<4>(U64_FORMAT_BUFFER, value, numDigits, is_upper(specs.Type));
                                this->write_no_specs(p, U64_FORMAT_BUFFER + numDigits - p);
                            },
                            formattedSize);
    } else if (type == 'n') {
        formattedSize += ((numDigits - 1) / 3);
        write_padded_helper(this, specs,
                            [&]() {
                                if (prefix.Length) this->write_no_specs(prefix);
                                For(range(padding)) this->write_no_specs(specs.Fill);
                                auto *p = format_uint_decimal(U64_FORMAT_BUFFER, value, formattedSize, "," /*@Locale*/);
                                this->write_no_specs(p, U64_FORMAT_BUFFER + formattedSize - p);
                            },
                            formattedSize);
    } else {
        assert(false && "Invalid type");  // sanity
    }
}

// Writes a float with given formatting specs
void format_context::write_f64(f64 value, format_specs specs) {
    byte type = specs.Type;
    if (type) {
        byte lower = (byte) to_lower(type);
        if (lower != 'g' && lower != 'e' && lower != '%' && lower != 'f' && lower != 'a') {
            on_error("Invalid type specifier");
            return;
        }
    } else {
        type = 'g';
    }

    bool percentage = specs.Type == '%';

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
        write_padded_helper(this, specs,
                            [&, this]() {
                                if (sign) this->write_no_specs(sign);
                                this->write_no_specs((bits.U & ((1ll << 52) - 1))
                                                         ? (is_upper(specs.Type) ? "NAN" : "nan")
                                                         : (is_upper(specs.Type) ? "INF" : "inf"));
                                if (percentage) this->write_no_specs(U'%');
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
    format_float(
        [](void *user, byte *buf, size_t length) {
            auto *fb = (stack_dynamic_buffer<512> *) user;
            fb->ByteLength += length;
            return fb->Data + fb->ByteLength;
        },
        &formatBuffer, formatBuffer.Data, type, value, specs.Precision);

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

            char *where = p;
            while (p != end && *p == '0') ++p;

            if (p == end || !is_digit(*p)) {
                if (p != end) copy_memory(where, p, (size_t)(end - p));
                formatBuffer.ByteLength -= (size_t)(p - where);
            }
        } else if (p == end) {
            // There was no dot at all
            formatBuffer.append_pointer_and_size(".0", 2);
        }
    }

    if (percentage) formatBuffer.append('%');

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

    auto formattedSize = formatBuffer.ByteLength + (sign ? 1 : 0);
    write_padded_helper(this, specs,
                        [&, this]() {
                            if (sign) this->write_no_specs(sign);
                            this->write_no_specs(formatBuffer.Data, formatBuffer.ByteLength);
                        },
                        formattedSize);
}

struct width_checker {
    format_context *F;

    template <typename T>
    enable_if_t<is_integer_v<T>, u32> operator()(T value) {
        if (IS_NEG(value)) {
            F->on_error("Negative width");
            return (u32) -1;
        } else if ((u64) value > numeric_info<s32>::max()) {
            F->on_error("Width value is too big");
            return (u32) -1;
        }
        return (u32) value;
    }

    template <typename T>
    enable_if_t<!is_integer_v<T>, u32> operator()(T) {
        F->on_error("Width was not an integer");
        return (u32) -1;
    }
};

struct precision_checker {
    format_context *F;

    template <typename T>
    enable_if_t<is_integer_v<T>, s32> operator()(T value) {
        if (IS_NEG(value)) {
            F->on_error("Negative precision");
            return numeric_info<s32>::min();
        } else if ((u64) value > numeric_info<s32>::max()) {
            F->on_error("Precision value is too big");
            return numeric_info<s32>::min();
        }
        return (s32) value;
    }

    template <typename T>
    enable_if_t<!is_integer_v<T>, s32> operator()(T) {
        F->on_error("Precision was not an integer");
        return numeric_info<s32>::min();
    }
};

arg format_context::get_arg_from_ref(arg_ref ref) {
    arg target;
    if (ref.Kind != arg_ref::kind::NONE) {
        if (ref.Kind == arg_ref::kind::INDEX) {
            if (ref.Index < Args.Count) {
                target = Args.get_arg(ref.Index);
            } else {
                on_error("Argument index out of range");
            }
        } else {
            ArgMap.ensure_initted(Args);
            target = ArgMap.find(ref.Name);
            if (target.Type == type::NONE) {
                --Parse.It;
                on_error("Argument with this name not found");
            }
        }
    }
    return target;
}

bool format_context::handle_dynamic_specs() {
    assert(Specs);

    auto width = get_arg_from_ref(Specs->WidthRef);
    if (width.Type != type::NONE) Specs->Width = visit_fmt_arg(width_checker{this}, width);
    if (Specs->Width == (u32) -1) return false;

    auto precision = get_arg_from_ref(Specs->PrecisionRef);
    if (precision.Type != type::NONE) Specs->Precision = visit_fmt_arg(precision_checker{this}, precision);
    if (Specs->Precision == numeric_info<s32>::min()) return false;

    return true;
}
}  // namespace fmt

LSTD_END_NAMESPACE
