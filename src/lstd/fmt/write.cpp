#include "lstd/fmt.h"

LSTD_BEGIN_NAMESPACE

void write_u64(fmt_context *f, u64 value, bool negative, fmt_specs specs);
void write_float(fmt_context *f, is_floating_point auto value, fmt_specs specs);

inline void write(fmt_context *f, is_integral auto value)
{
    u64 absValue = (u64)value;
    bool negative = sign_bit(value);
    if (negative)
        absValue = 0 - absValue;

    if (f->Specs)
    {
        write_u64(f, absValue, negative, *f->Specs);
    }
    else
    {
        write_u64(f, absValue, negative, {});
    }
}

inline void write(fmt_context *f, is_floating_point auto value)
{
    if (f->Specs)
    {
        write_float(f, value, *f->Specs);
    }
    else
    {
        write_float(f, value, {});
    }
}

inline void write_no_specs(fmt_context *f, is_integral auto value)
{
    u64 absValue = (u64)value;
    bool negative = sign_bit(value);
    if (negative)
        absValue = 0 - absValue;
    write_u64(f, absValue, negative, {});
}

inline void write_no_specs(fmt_context *f, is_floating_point auto value)
{
    write_float(f, (f64)value, {});
}

inline void write_no_specs(fmt_context *f, bool value)
{
    write_no_specs(f, value ? 1 : 0);
}

inline void write_no_specs(fmt_context *f, const void *value)
{
    auto *old = f->Specs;
    f->Specs = null;
    write(f, value);
    f->Specs = old;
}

inline char FORMAT_UINT_DIGITS[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

template <typename UInt>
char *format_uint_decimal(char *buffer, UInt value, s64 formattedSize,
                          string thousandsSep = "")
{
    u32 digitIndex = 0;

    buffer += formattedSize;
    while (value >= 100)
    {
        u32 index = (u32)(value % 100) * 2;
        value /= 100;
        *--buffer = FORMAT_UINT_DIGITS[index + 1];
        if (++digitIndex % 3 == 0)
        {
            buffer -= thousandsSep.Count;
            memcpy(buffer, thousandsSep.Data, thousandsSep.Count);
        }
        *--buffer = FORMAT_UINT_DIGITS[index];
        if (++digitIndex % 3 == 0)
        {
            buffer -= thousandsSep.Count;
            memcpy(buffer, thousandsSep.Data, thousandsSep.Count);
        }
    }

    if (value < 10)
    {
        *--buffer = (char)('0' + value);
        return buffer;
    }

    u32 index = (u32)value * 2;
    *--buffer = FORMAT_UINT_DIGITS[index + 1];
    if (++digitIndex % 3 == 0)
    {
        buffer -= thousandsSep.Count;
        memcpy(buffer, thousandsSep.Data, thousandsSep.Count);
    }
    *--buffer = FORMAT_UINT_DIGITS[index];

    return buffer;
}

template <u32 BASE_BITS, typename UInt>
char *format_uint_base(char *buffer, UInt value, s64 formattedSize,
                       bool upper = false)
{
    buffer += formattedSize;
    do
    {
        const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
        u32 digit = (value & ((1 << BASE_BITS) - 1));
        *--buffer = (char)(BASE_BITS < 4 ? (char)('0' + digit) : digits[digit]);
    } while ((value >>= BASE_BITS) != 0);
    return buffer;
}

// @Cleanup Replace lambda with string...

// Writes pad code points and the actual contents with f(),
// _fSize_ needs to be the size of the output from _f_ in code points (in order
// to calculate padding properly)
template <typename F>
void write_padded_helper(fmt_context *f, const fmt_specs &specs, F &&func,
                         s64 fSize)
{
    u32 padding = (u32)(specs.Width > fSize ? specs.Width - fSize : 0);
    if (specs.Align == fmt_alignment::RIGHT)
    {
        For(range(padding)) write_no_specs(f, specs.Fill);
        func();
    }
    else if (specs.Align == fmt_alignment::CENTER)
    {
        u32 leftPadding = padding / 2;
        For(range(leftPadding)) write_no_specs(f, specs.Fill);
        func();
        For(range(padding - leftPadding)) write_no_specs(f, specs.Fill);
    }
    else
    {
        func();
        For(range(padding)) write_no_specs(f, specs.Fill);
    }
}

inline void write_helper(fmt_context *f, const char *data, s64 size)
{
    if (!f->Specs)
    {
        write_no_specs(f, data, size);
        return;
    }

    // Helper to compute truncation and ellipsis application for UTF-8 strings
    struct _string_truncation_result
    {
        s64 length;       // visible code points (excluding ellipsis)
        s64 sizeBytes;    // byte count to output from data
        bool addEllipsis; // whether to append "..."
    };
    auto _compute_truncation = [&](const char *s, s64 n, s64 precision) -> _string_truncation_result
    {
        _string_truncation_result r{};
        r.length = utf8_length(s, n);
        r.sizeBytes = n;
        r.addEllipsis = false;
        if (precision != -1 && precision < r.length)
        {
            s64 target = precision;
            if (target >= 4)
            {
                target -= 3;
                r.addEllipsis = true;
            }
            else if (target <= 0)
            {
                r.length = 0;
                r.sizeBytes = 0;
                r.addEllipsis = false;
                return r;
            }
            r.length = target;
            r.sizeBytes = utf8_get_pointer_to_cp_at_translated_index(s, n, target) - s;
        }
        return r;
    };

    if (f->Specs->Type)
    {
        if (f->Specs->Type == 'p')
        {
            write(f, (const void *)data);
            return;
        }
        if (f->Specs->Type == 'q')
        {
            // Quoted string format with precision applied to the inner content.
            auto tr = _compute_truncation(data, size, f->Specs->Precision);
            // Approx visible width: quotes + inner content + optional ellipsis
            s64 approx_visible = 2 + tr.length + (tr.addEllipsis ? 3 : 0);
            write_padded_helper(
                f, *f->Specs,
                [&]()
                {
                    write_no_specs(f, "\"");
                    for (s64 i = 0; i < tr.sizeBytes; ++i)
                    {
                        char c = data[i];
                        switch (c)
                        {
                        case '"':
                            write_no_specs(f, "\\\"");
                            break;
                        case '\\':
                            write_no_specs(f, "\\\\");
                            break;
                        case '\n':
                            write_no_specs(f, "\\n");
                            break;
                        case '\r':
                            write_no_specs(f, "\\r");
                            break;
                        case '\t':
                            write_no_specs(f, "\\t");
                            break;
                        default:
                            write_no_specs(f, &c, 1);
                            break;
                        }
                    }
                    if (tr.addEllipsis)
                        write_no_specs(f, "...");
                    write_no_specs(f, "\"");
                },
                approx_visible);
            return;
        }
        if (f->Specs->Type != 's')
        {
            f->on_error( "Invalid type specifier for a string",
                     f->Parse.It.Data - f->Parse.FormatString.Data - 1);
            return;
        }
    }

    // 'p' wasn't specified, not treating as formatting a pointer
    auto tr = _compute_truncation(data, size, f->Specs->Precision);
    write_padded_helper(
        f, *f->Specs,
        [&]()
        {
            write_no_specs(f, data, tr.sizeBytes);
            if (tr.addEllipsis)
                write_no_specs(f, "...");
        },
        tr.length + (tr.addEllipsis ? 3 : 0));
}

inline void fmt_context::write(const char *data, s64 count)
{
    write_helper(this, data, count);
}

inline void write(fmt_context *f, bool value)
{
    if (f->Specs && f->Specs->Type)
    {
        write(f, value ? 1 : 0);
    }
    else
    {
        write(f, string(value ? "true" : "false"));
    }
}

inline void write(fmt_context *f, const void *value)
{
    if (f->Specs && f->Specs->Type && f->Specs->Type != 'p')
    {
        f->on_error( "Invalid type specifier for a pointer",
                 f->Parse.It.Data - f->Parse.FormatString.Data - 1);
        return;
    }

    auto uptr = bit_cast<u64>(value);
    u32 numDigits = count_digits<4>(uptr);

    auto func = [&, f]()
    {
        write_no_specs(f, U'0');
        write_no_specs(f, U'x');

        char formatBuffer[numeric<u64>::digits / 4 + 2];
        auto *p = format_uint_base<4>(formatBuffer, uptr, numDigits);
        write_no_specs(f, p, formatBuffer + numDigits - p);
    };

    if (!f->Specs)
    {
        func();
        return;
    }

    fmt_specs specs = *f->Specs;
    if (specs.Align == fmt_alignment::NONE)
        specs.Align = fmt_alignment::RIGHT;
    write_padded_helper(f, specs, func, numDigits + 2);
}

inline void write_u64(fmt_context *f, u64 value, bool negative,
                      fmt_specs specs)
{
    char type = specs.Type;
    if (!type)
        type = 'd';

    s64 numDigits;
    if (type == 'd' || type == 'n')
    {
        numDigits = count_digits(value);
    }
    else if (ascii_to_lower(type) == 'b')
    {
        numDigits = count_digits<1>(value);
    }
    else if (type == 'o')
    {
        numDigits = count_digits<3>(value);
    }
    else if (ascii_to_lower(type) == 'x')
    {
        numDigits = count_digits<4>(value);
    }
    else if (type == 'c')
    {
        if (specs.Align == fmt_alignment::NUMERIC || specs.Sign != fmt_sign::NONE ||
            specs.Hash)
        {
            f->on_error(
                     "Invalid format specifier(s) for code point - code points can't "
                     "have numeric alignment, signs or #",
                     f->Parse.It.Data - f->Parse.FormatString.Data);
            return;
        }
        auto cp = (code_point)value;
        write_padded_helper(
            f, specs, [&]()
            { write_no_specs(f, cp); }, 1);
        return;
    }
    else
    {
        f->on_error( "Invalid type specifier for an integer",
                 f->Parse.It.Data - f->Parse.FormatString.Data - 1);
        return;
    }

    if (value == 0)
    {
        numDigits = 1;
    }

    char prefixBuffer[4];
    char *prefixPointer = prefixBuffer;

    if (negative)
    {
        *prefixPointer++ = '-';
    }
    else if (specs.Sign == fmt_sign::PLUS)
    {
        *prefixPointer++ = '+';
    }
    else if (specs.Sign == fmt_sign::SPACE)
    {
        *prefixPointer++ = ' ';
    }

    if ((ascii_to_lower(type) == 'x' || ascii_to_lower(type) == 'b') && specs.Hash)
    {
        *prefixPointer++ = '0';
        *prefixPointer++ = type;
    }

    // Octal prefix '0' is counted as a digit,
    // so only add it if precision is not greater than the number of digits.
    if (type == 'o' && specs.Hash)
    {
        if (specs.Precision == -1 || specs.Precision > numDigits)
            *prefixPointer++ = '0';
    }

    auto prefix = string(prefixBuffer, prefixPointer - prefixBuffer);
    auto prefixLength = length(prefix);

    s64 formattedSize = prefixLength + numDigits;
    s64 padding = 0;
    if (specs.Align == fmt_alignment::NUMERIC)
    {
        if (specs.Width > formattedSize)
        {
            padding = specs.Width - formattedSize;
            formattedSize = specs.Width;
        }
    }
    else if (specs.Precision > numDigits)
    {
        formattedSize = (u32)prefixLength + (u32)specs.Precision;
        padding = (u32)specs.Precision - numDigits;
        specs.Fill = '0';
    }
    if (specs.Align == fmt_alignment::NONE)
        specs.Align = fmt_alignment::RIGHT;

    char U64_FORMAT_BUFFER[numeric<u64>::digits + 1]{};

    if (type == 'n')
    {
        formattedSize += ((numDigits - 1) / 3);
    }

    type = ascii_to_lower(type);
    write_padded_helper(
        f, specs,
        [&]()
        {
            if (prefix.Count)
                write_no_specs(f, prefix);
            For(range(padding)) write_no_specs(f, specs.Fill);

            char *p = null;
            if (type == 'd')
            {
                p = format_uint_decimal(U64_FORMAT_BUFFER, value, numDigits);
            }
            else if (type == 'b')
            {
                p = format_uint_base<1>(U64_FORMAT_BUFFER, value, numDigits);
            }
            else if (type == 'o')
            {
                p = format_uint_base<3>(U64_FORMAT_BUFFER, value, numDigits);
            }
            else if (type == 'x')
            {
                p = format_uint_base<4>(U64_FORMAT_BUFFER, value, numDigits,
                                        ascii_is_upper(specs.Type));
            }
            else if (type == 'n')
            {
                numDigits = formattedSize; // To include extra chars (like commas)
                p = format_uint_decimal(U64_FORMAT_BUFFER, value, formattedSize,
                                        "," /*@Locale*/);
            }
            else
            {
                assert(false && "Invalid type"); // sanity
            }

            write_no_specs(f, p, U64_FORMAT_BUFFER + numDigits - p);
        },
        formattedSize);
}

// Writes the exponent exp in the form "[+-]d{2,3}"
inline void write_exponent(fmt_context *f, s64 exp)
{
    assert(-10000 < exp && exp < 10000);

    if (exp < 0)
    {
        write_no_specs(f, U'-');
        exp = -exp;
    }
    else
    {
        write_no_specs(f, U'+');
    }

    if (exp >= 100)
    {
        auto *top = &FORMAT_UINT_DIGITS[exp / 100 * 2];
        if (exp >= 1000)
            write_no_specs(f, (code_point)(top[0]));
        write_no_specs(f, (code_point)(top[1]));
        exp %= 100;
    }

    auto *d = &FORMAT_UINT_DIGITS[exp * 2];
    write_no_specs(f, (code_point)(d[0]));
    write_no_specs(f, (code_point)(d[1]));
}

// Routine to write the formatted significant including a decimalPoint if
// necessary
//
// @Robustness
// We assume significand contains only ASCII 0-9 digits
// and significand.Count == length(significand).
// I'm not sure if we will ever format anything besides
// arabic numerals so ...
//
inline void write_significand(fmt_context *f, string significand, s64 integralSize, code_point decimalPoint = 0)
{
    if (!significand.Count)
        return; // The significand is actually empty if the value formatted is 0

    write_no_specs(f, slice(significand, 0, integralSize));
    if (decimalPoint)
    {
        write_no_specs(f, decimalPoint);
        write_no_specs(f, slice(significand, integralSize, significand.Count));
    }
}

// Routine to write a float in EXP format
inline void write_float_exp(fmt_context *f, string significand, s32 exp,
                            code_point sign, fmt_specs no_copy specs,
                            fmt_float_specs no_copy floatSpecs)
{
    s64 outputSize = (sign ? 1 : 0) +
                     significand.Count; // Further we add the number of zeros/the
                                        // size of the exponent to this tally

    code_point decimalPoint =
        '.'; // @Locale... Also if we decide to add a thousands separator?

    s64 numZeros = 0;
    if (floatSpecs.ShowPoint)
    {
        numZeros = specs.Precision - significand.Count;
        if (numZeros < 0)
            numZeros = 0;
        outputSize += numZeros;
    }
    else if (significand.Count == 1)
    {
        decimalPoint = 0;
    }

    // Convert exp to the first digit
    exp += (s32)(significand.Count - 1);

    //
    // Choose 2, 3 or 4 exponent digits depending on the magnitude
    s64 absExp = abs(exp);

    s32 expDigits = 2;
    if (absExp >= 100)
        expDigits = absExp >= 1000 ? 4 : 3;

    outputSize +=
        (decimalPoint ? 1 : 0) + 2 + expDigits; // +2 bytes for "[+-][eE]"

    code_point expChar = floatSpecs.Upper ? 'E' : 'e';

    write_padded_helper(
        f, specs,
        [&]()
        {
            if (sign)
                write_no_specs(f, sign);

            // Write significand, then the zeroes (if required by the precision),
            // then the exp char and then the exponent itself e.g. 1.23400e+5
            write_significand(f, significand, 1, decimalPoint);
            For(range(numZeros)) write_no_specs(f, U'0');
            write_no_specs(f, expChar);
            write_exponent(f, exp);
        },
        outputSize);
    return;
}

// Routine to write a float in FIXED format
inline void write_float_fixed(fmt_context *f, string significand, s32 exp,
                              code_point sign, const fmt_specs &specs,
                              const fmt_float_specs &floatSpecs,
                              bool percentage)
{
    s64 outputSize =
        (sign ? 1 : 0) + (percentage ? 1 : 0) +
        significand.Count; // Further down we add the number of extra
                           // zeros needed and the decimal point

    code_point decimalPoint =
        '.'; // @Locale... Also if we decide to add a thousands separator?

    if (exp >= 0)
    {
        // Case: 1234e5 -> 123400000[.0+]

        outputSize += exp;

        // Determine how many zeros we need to add after the decimal point to match
        // the precision, note that this is different than the zeroes we add BEFORE
        // the decimal point that are needed to match the magnitude of the number.
        s64 numZeros = decimalPoint ? specs.Precision - exp : 0;

        if (floatSpecs.ShowPoint)
        {
            //
            // :PythonLikeConsistency:
            // If we are going formatting with implicit spec,
            // and there was no specified precision, we add 1 trailing zero,
            // e.g. "{}", 42 -> gets formatted to: "42.0"
            //
            // This is done so we are consistent with the Python style of formatting
            // floats.
            //

            if (numZeros <= 0 && floatSpecs.Format != fmt_float_specs::FIXED)
                numZeros = 1;
            if (numZeros > 0)
                outputSize += numZeros + 1; // +1 for the dot
        }

        write_padded_helper(
            f, specs,
            [&]()
            {
                if (sign)
                    write_no_specs(f, sign);

                write_significand(
                    f, significand,
                    significand.Count); // Write the whole significand, without
                                        // putting the dot anywhere
                For(range(exp)) write_no_specs(
                    f, U'0'); // Add any needed zeroes to match the magnitude

                // Add the decimal point if needed
                if (floatSpecs.ShowPoint)
                {
                    write_no_specs(f, decimalPoint);
                    For(range(numZeros)) write_no_specs(f, U'0');
                }
                if (percentage)
                    write_no_specs(f, U'%');
            },
            outputSize);
    }
    else if (exp < 0)
    {
        s64 absExp = abs(exp);

        if (absExp < significand.Count)
        {
            // Case: 1234e-2 -> 12.34[0+]

            s64 numZeros = floatSpecs.ShowPoint ? specs.Precision - absExp : 0;
            outputSize += 1 + (numZeros > 0 ? numZeros : 0);

            write_padded_helper(
                f, specs,
                [&]()
                {
                    if (sign)
                        write_no_specs(f, sign);

                    // The decimal point is positioned at _absExp_ symbols before the
                    // end of the significand
                    s64 decimalPointPos = significand.Count - absExp;

                    // Write the significand, then write any zeroes if needed (for the
                    // precision)
                    write_significand(f, significand, decimalPointPos, decimalPoint);
                    For(range(numZeros)) write_no_specs(f, U'0');
                    if (percentage)
                        write_no_specs(f, U'%');
                },
                outputSize);
        }
        else
        {
            // Case: 1234e-6 -> 0.001234

            // We know that absExp >= significand.Count
            s64 numZeros = absExp - significand.Count;

            // Edge case when we are formatting a 0 with given precision
            if (!significand.Count && specs.Precision >= 0 &&
                specs.Precision < numZeros)
            {
                numZeros = specs.Precision;
            }

            bool pointy = numZeros || significand.Count || floatSpecs.ShowPoint;
            outputSize += 1 + (pointy ? 1 : 0) + numZeros;

            write_padded_helper(
                f, specs,
                [&]()
                {
                    if (sign)
                        write_no_specs(f, sign);

                    write_no_specs(f, U'0');

                    if (pointy)
                    {
                        // Write the decimal point + the zeros + the significand
                        write_no_specs(f, decimalPoint);
                        For(range(numZeros)) write_no_specs(f, U'0');

                        write_significand(f, significand, significand.Count);
                    }
                    if (percentage)
                        write_no_specs(f, U'%');
                },
                outputSize);
        }
    }
}

inline fmt_float_specs fmt_parse_float_specs(fmt_parse_context *p,
                                             fmt_specs no_copy specs) {
  fmt_float_specs result;

  result.ShowPoint = specs.Hash;
  result.Upper = false;

  switch (specs.Type) {
    case 0:
      result.Format = fmt_float_specs::GENERAL;
      // result.ShowPoint = true;  // :PythonLikeConsistency: See other note
      // with this tag in fmt.context.ixx
      break;
    case 'G':
      result.Upper = true;
      [[fallthrough]];
    case 'g':
      result.Format = fmt_float_specs::GENERAL;
      break;
    case 'E':
      result.Upper = true;
      [[fallthrough]];
    case 'e':
      result.Format = fmt_float_specs::EXP;
      result.ShowPoint |= specs.Precision != 0;
      break;
    case 'F':
      result.Upper = true;
      [[fallthrough]];
    case '%':  // When the spec is '%' we display the number with fixed format
               // and multiply it by 100
      [[fallthrough]];
    case 'f':
      result.Format = fmt_float_specs::FIXED;
      result.ShowPoint |= specs.Precision != 0;
      break;
    case 'A':
      result.Upper = true;
      [[fallthrough]];
    case 'a':
      result.Format = fmt_float_specs::HEX;
      break;
    default:
      p->on_error("Invalid type specifier for a f32",
               p->It.Data - p->FormatString.Data - 1);
      break;
  }
  return result;
}

// Used to store a floating point number as F * pow(2, E), where F is the
// significand and E is the exponent. Used by both Dragonbox and Grisu.
template <is_floating_point F>
struct decimal_fp {
  using significand_t = type_select_t<sizeof(F) == sizeof(f32), u32, u64>;

  significand_t Significand;
  s32 Exponent;
  s32 MantissaBit;  // Required by dragon4
};

using fp = decimal_fp<f64>;

// Assigns _d_ to this and return true if predecessor is closer than successor
// (is the high margin twice as large as the low margin).
template <is_floating_point F>
bool fp_assign_new(fp &f, F newValue) {
  u64 implicitBit = 1ull << numeric<F>::bits_mantissa;
  u64 significandMask = implicitBit - 1;

  u64 exponentMask = ((1ull << numeric<F>::bits_exponent) - 1)
                     << numeric<F>::bits_mantissa;

  auto br =
      bit_cast<type_select_t<sizeof(F) == sizeof(f32), u32, u64>>(newValue);

  f.Significand = br & significandMask;
  s32 biasedExp = (s32)((br & exponentMask) >> numeric<F>::bits_mantissa);

  // Predecessor is closer if _f_ is a normalized power of 2 (f.Significand ==
  // 0) other than the smallest normalized number (biasedExp > 1).
  bool isPredecessorCloser = f.Significand == 0 && biasedExp > 1;

  if (biasedExp) {
    f.Significand += implicitBit;
    f.MantissaBit = numeric<F>::bits_mantissa;
  } else {
    biasedExp = 1;  // Subnormals use biased exponent 1 (min exponent).
    f.MantissaBit = msb(f.Significand | 1);  // Integer log2
  }
  f.Exponent =
      biasedExp - numeric<F>::exponent_bias - numeric<F>::bits_mantissa;

  return isPredecessorCloser;
}

// Normalizes the value converted from double and multiplied by (1 << SHIFT).
template <s32 SHIFT>
fp fp_normalize(fp value) {
  const u64 IMPLICIT_BIT = 1ull << numeric<f64>::bits_mantissa;

  // Handle subnormals.
  u64 shifted_implicit_bit = IMPLICIT_BIT << SHIFT;
  while ((value.Significand & shifted_implicit_bit) == 0) {
    value.Significand <<= 1;
    --value.Exponent;
  }

  // Subtract 1 to account for hidden bit.
  s32 offset =
      (s32)((sizeof(u64) * 8) - numeric<f64>::bits_mantissa - SHIFT - 1);
  value.Significand <<= offset;
  value.Exponent -= offset;
  return value;
}

always_inline fp operator*(fp x, fp y) {
  // Computes x.Significand * y.Significand / pow(2, 64) rounded to nearest with
  // half-up tie breaking.
  u128 product = u128(x.Significand) * y.Significand;

  u64 f = (u64)(product >> 64);
  x.Significand = ((u64)product & (1ull << 63)) != 0 ? f + 1 : f;

  x.Exponent += y.Exponent + 64;
  return x;
}

// Returns exponent base 10 of the last digit written; writes digits without a decimal point.
s32 fmt_format_non_negative_float(string_builder *floatBuffer,
                                  is_floating_point auto value,
                                  s32 precision,
                                  fmt_float_specs no_copy specs);

// Writes a float with given formatting specs
inline void write_float(fmt_context *f, is_floating_point auto value, fmt_specs specs)
{
    fmt_float_specs floatSpecs = fmt_parse_float_specs(&f->Parse, specs);

    //
    // Determine the sign
    //
    code_point sign = 0;

    // Check the sign bit instead of just checking "value < 0" since the latter is
    // always false for NaN
    if (sign_bit(value))
    {
        value = -value;
        sign = '-';
    }
    else
    {
        // value is positive
        if (specs.Sign == fmt_sign::PLUS)
        {
            sign = '+';
        }
        else if (specs.Sign == fmt_sign::SPACE)
        {
            sign = ' ';
        }
    }

    // When the spec is '%' we display the number with fixed format and multiply
    // it by 100. The spec gets handled in fmt_parse_float_specs().

    bool percentage = specs.Type == '%';
    if (percentage)
    {
        value *= 100;
    }

    //
    // Handle INF or NAN
    //
    if (!is_finite(value))
    {
        write_padded_helper(
            f, specs,
            [&]()
            {
                if (sign)
                    write_no_specs(f, sign);
                write_no_specs(f, is_nan(value)
                                      ? (ascii_is_upper(specs.Type) ? "NAN" : "nan")
                                      : (ascii_is_upper(specs.Type) ? "INF" : "inf"));
                if (percentage)
                    write_no_specs(f, U'%');
            },
            3 + (sign ? 1 : 0) + (percentage ? 1 : 0));
        return;
    }

    if (floatSpecs.Format == fmt_float_specs::HEX)
    {
        // @TODO Hex floats
        return;
    }

    // Default precision we do for floats is 6 (except if the spec type is none)
    if (specs.Precision < 0 && specs.Type)
        specs.Precision = 6;

    if (floatSpecs.Format == fmt_float_specs::EXP && specs.Precision != 0)
    {
        if (specs.Precision == numeric<s32>::max())
        {
            f->on_error( "Number too big");
            return;
        }
        ++specs.Precision;
    }

    //
    // Handle alignment NUMERIC or NONE
    //
    if (specs.Align == fmt_alignment::NUMERIC)
    {
        if (sign)
        {
            write_no_specs(f, sign);
            sign = 0;
            if (specs.Width)
                --specs.Width;
        }
        specs.Align = fmt_alignment::RIGHT;
    }
    else if (specs.Align == fmt_alignment::NONE)
    {
        specs.Align = fmt_alignment::RIGHT;
    }

    // This routine writes the significand in the floatBuffer, then we use the
    // returned exponent to choose how to format the final string. The returned
    // exponent is the exponent base 10 of the LAST written digit in
    // _floatBuffer_.
    string_builder floatBuffer;
    s32 exp = fmt_format_non_negative_float(&floatBuffer, value, specs.Precision, floatSpecs);

    //
    // Assert we haven't allocated, which would be bad, because our formatting
    // library is not supposed to allocate by itself.
    //
    // Note that string builder allocates if the default buffer (size 1 KiB) runs
    // out of space, would anybody even try to format such a big float?
    //
    // I'm more into the idea to say "f it" and just don't handle the case when
    // the formatted float is bigger (truncate), instead of adding to the
    // documentation "Hey, by the way, formatting floats may allocate memory."
    //
    // @TODO: Make string_builder not add additional buffers with an option (maybe
    // a template?).
    //
    assert(!floatBuffer.IndirectionCount);

    string significand = string((char *)floatBuffer.BaseBuffer.Data,
                                floatBuffer.BaseBuffer.Occupied);

    s64 outputExp = exp + significand.Count - 1;

    bool useExpFormat = false;
    if (floatSpecs.Format == fmt_float_specs::EXP)
    {
        useExpFormat = true;
    }
    else if (floatSpecs.Format == fmt_float_specs::GENERAL)
    {
        // If we are using the general format, we use the fixed notation (0.0001) if
        // the exponent is in [EXP_LOWER, EXP_UPPER/precision), instead of the
        // exponent notation (1e-04) in the other case.
        const s64 EXP_LOWER = -4;
        const s64 EXP_UPPER = 16;

        // We also pay attention if the precision has been set.
        // By the time we get here it can be -1 for the general format (if the user
        // hasn't specified a precision).
        useExpFormat =
            outputExp < EXP_LOWER ||
            outputExp >= (specs.Precision > 0 ? specs.Precision : EXP_UPPER);
    }

    if (useExpFormat)
    {
        write_float_exp(f, significand, exp, sign, specs, floatSpecs);
    }
    else
    {
        write_float_fixed(f, significand, exp, sign, specs, floatSpecs, percentage);
    }
}

LSTD_END_NAMESPACE