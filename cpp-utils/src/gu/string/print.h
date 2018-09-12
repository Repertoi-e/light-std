#pragma once

#include "../memory/array.h"
#include "../memory/temporary_allocator.h"
#include "string_builder.h"

#include <tuple>
#include <type_traits>

namespace private_print {
inline const char g_NumberBaseCharacters[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";

inline void print_u64_with_format_to_builder(String_Builder &builder, u64 v, u64 base, s32 digits) {
    const s32 MAX_OUTPUT_LENGTH = 64;
    if (digits > MAX_OUTPUT_LENGTH) {
        digits = MAX_OUTPUT_LENGTH;
    }
    if (digits == 0) {
        return;  // Do not print anything!
    }

    char output[MAX_OUTPUT_LENGTH];
    char *end = &output[MAX_OUTPUT_LENGTH];
    char *p = end;

    while (v || (digits > 0)) {
        u64 place = v % base;
        v /= base;
        p--;
        *p = g_NumberBaseCharacters[place];
        digits--;
    }

    if ((p == end) && ((digits > 0) || (digits == -1))) {
        p--;
        *p = '0';
    }

    assert(p >= output);

    append_pointer_and_size(builder, p, end - p);
}

inline const f64 g_PowersOf10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

// TODO: More formatting options
inline void print_f64_with_format_to_builder(String_Builder &builder, f64 value, s32 precision, s32 width) {
    // NOTE: We assume the formatted float fits in 64 chars (width padding not
    // included, as that is dirrectly added to the builder)
    const s32 MAX_OUTPUT_LENGTH = 64;

    char output[MAX_OUTPUT_LENGTH];
    char *end = &output[MAX_OUTPUT_LENGTH];
    char *p = end;

    // Handle negative
    b32 negative = false;
    if (value < 0) {
        negative = true;
        value = -value;
    }

    // If input is larger than 0x7FFFFFFF, fail
    // TODO: We need to do exponential here
    if (value > 0x7FFFFFFF) {
        append_cstring(builder, "{Float too big}");
        return;
    }

    if (precision > 9U) {
        precision = 9U;
    }

    s32 whole = (s32) value;
    f64 tmp = (value - whole) * g_PowersOf10[precision];
    u32 fraction = (u32) tmp;
    f64 diff = tmp - fraction;

    if (diff > 0.5) {
        ++fraction;
        // handle rollover, e.g. case 0.99 with prec 1 is 1.0
        if (fraction >= g_PowersOf10[precision]) {
            fraction = 0;
            ++whole;
        }
    } else if ((diff == 0.5) && ((fraction == 0U) || (fraction & 1U))) {
        // If halfway, round up if odd, OR if last digit is 0
        ++fraction;
    }

    if (precision == 0U) {
        diff = value - (f64) whole;
        if (diff > 0.5) {
            // greater than 0.5, round up, e.g. 1.6 -> 2
            ++whole;
        } else if ((diff == 0.5) && (whole & 1)) {
            // exactly 0.5 and ODD, then round up
            // 1.5 -> 2, but 2.5 -> 2
            ++whole;
        }
    } else {
        u32 count = precision;
        // Now do fractional part, as an unsigned number
        do {
            --count;
            p--;
            *p = (char) ('0' + (fraction % 10U));
        } while (fraction /= 10U);
        // Add extra 0s
        while (count-- > 0U) {
            p--;
            *p = '0';
        }
        p--;
        *p = '.';
    }

    // Do whole part
    if (!whole) {
        p--;
        *p = '0';
    } else {
        while (whole) {
            p--;
            *p = (char) ('0' + (whole % 10));
            whole /= 10;
        }
    }

    // TODO: More formatting options
    if (negative) {
        p--;
        *p = '-';
    }

    // Pad spaces up to given width
    // TODO: More formatting options
    for (size_t i = end - p; i < width; i++) {
        append_pointer_and_size(builder, " ", 1);
    }

    append_pointer_and_size(builder, p, end - p);
}
}  // namespace private_print

inline constexpr u32 TO_STRING_FLOAT_SPACE = 1;

// TODO: Maybe support more formatting options, but IDK a good way to handle
// them.
//    If we use just parameters it would become very cryptic on the caller's
//    side:
//
//    to_string(2.5, -4, 5, true, false, yes, 10);
//
// (Maybe do the Named Parameter Idiom)
template <typename T>
inline typename std::enable_if_t<std::is_floating_point_v<T>, string> to_string(T v, s32 width = 0, s32 precision = 6) {
    String_Builder builder;
    private_print::print_f64_with_format_to_builder(builder, (f64) v, precision, width);
    return to_string(builder);
}

struct Base {
    s32 Radix;
    explicit Base(s32 base) : Radix(base) {}
};

template <typename T>
inline typename std::enable_if_t<std::is_integral_v<T>, string> to_string(T v, Base base = Base(10),
                                                                          s32 minimumDigits = -1) {
    assert(base.Radix >= 2 && "Invalid base");
    assert(base.Radix <= 64 && "Invalid base");

    String_Builder builder;

    u64 value = v;
    if constexpr (std::is_signed_v<T>) {
        if (v < 0) {
            append_cstring(builder, "-");
            value = (u64)(-v);
        } else {
            value = (u64) v;
        }
    } else {
        value = (u64) v;
    }
    private_print::print_u64_with_format_to_builder(builder, value, base.Radix, minimumDigits);

    return to_string(builder);
}

template <typename T>
inline typename std::enable_if_t<std::is_same_v<T, bool>, string> to_string(T v) {
    return v ? "true" : "false";
}

// Width means the length of the formatted string.
// If the input is larger than the width, it gets cut with an ellipsis:
//      this_string_is_too_lo...
// If the string is shorter than the widht it is padded to the left. Example:
// (width = 18)
//
//      Hello, world!     \0
//      ^^^^^^^^^^^^^^^^^^
// To pad to the right use negative width. Example: (width = -18)
//
//           Hello, world!\0
//      ^^^^^^^^^^^^^^^^^^
inline string to_string(string const &v, s32 width = 0) {
    String_Builder builder;

    s32 positiveWidth = width > 0 ? width : -width;

    const char *stringStart = v.Data;
    size_t stringSize = v.CountBytes;
    size_t len = length(v);

    // If the width is 1 or 2, adding 3 dots exceeds it,
    // so we add as much as needed without exceeding it.
    s32 ellipsis = 3;
    if (ellipsis > positiveWidth) {
        ellipsis = positiveWidth;
    }

    if (width < 0) {
        if (len > positiveWidth) {
            for (s32 i = 0; i < ellipsis; i++) {
                append_cstring(builder, ".");
            }
            while (positiveWidth - ellipsis != len--) {
                s32 eatCodePoint;
                utf8codepoint(stringStart, &eatCodePoint);

                size_t codePointSize = utf8codepointsize(eatCodePoint);
                stringStart += codePointSize;
                stringSize -= codePointSize;
            }
        } else {
            // Add needed spaces
            while (positiveWidth != len++) {
                append_cstring(builder, " ");
            }
        }
    }
    s32 appendEllipsisAfter = 0;
    s32 appendSpacesAfter = 0;
    if (width > 0) {
        if (len > width) {
            appendEllipsisAfter = ellipsis;
            while (width - ellipsis != len--) {
                s32 eatCodePoint;
                utf8codepoint(stringStart, &eatCodePoint);
                stringSize -= utf8codepointsize(eatCodePoint);
            }
        } else {
            appendSpacesAfter = (s32) (width - len);
        }
    }
    append_pointer_and_size(builder, stringStart, stringSize);
    while (appendEllipsisAfter--) {
        append_cstring(builder, ".");
    }
    while (appendSpacesAfter--) {
        append_cstring(builder, " ");
    }

    return to_string(builder);
}

// TODO: parse_float, parse_bool, etc.
inline std::tuple<s64, size_t> parse_number(const char *str) {
    s64 result = 0;

    b32 negative = false;
    if (*str == '-') {
        negative = true;
    }

    size_t length = 0;
    while (is_digit(*str)) {
        result *= 10;
        result += *str - '0';

        str++;
        length++;
    }

    if (negative) {
        return {-result, length + 1};
    } else {
        return {result, length};
    }
}

// Print formatted string to String Builder
template <typename... Args>
inline void print_to_builder(String_Builder &builder, string const &format, Args &&... argsPack) {
	if constexpr (sizeof...(argsPack) > 0) {
		Array<string, sizeof...(argsPack)> args = { {to_string(std::forward<Args>(argsPack))...} };

		size_t implicitArgIndex = 0;
		size_t cursor = 0, printed = 0;

		while (cursor < length(format)) {
			char32_t c = format[cursor];
			if (c != '%') {
				cursor++;
				continue;
			}

			append_pointer_and_size(builder, format.Data + printed, cursor - printed);
			cursor++;  // Skip the %

			char32_t next = format[cursor];
			if (next == '%') {
				// Double-percent means to actually output a percent.
				append_cstring(builder, "%");
				cursor++;
				printed = cursor;
				continue;
			}

			size_t argIndex = 0;
			if (!is_digit(next)) {
				argIndex = implicitArgIndex;
			} else {
				auto[number, length] = parse_number(format.Data + cursor);
				argIndex = number - 1;
				cursor += length;
			}

			if (argIndex < args.Count) {
				append(builder, args[argIndex]);
				implicitArgIndex = argIndex + 1;
			} else {
				append_cstring(builder, "{Invalid format argument}");
			}

			printed = cursor;
		}

		append_pointer_and_size(builder, format.Data + printed, cursor - printed);
	} else {
		append(builder, format);
	}
}

// Format a string
template <typename... Args>
inline string sprint(string const &format, Args &&... argsPack) {
    String_Builder builder;
    print_to_builder(builder, format, std::forward<Args>(argsPack)...);
    return to_string(builder);
}

// Format a string using the temporary allocator (if it has been initialized)
template <typename... Args>
inline string tprint(string const &format, Args &&... argsPack) {
    Allocator_Closure oldAllocator;
    if (__temporary_allocator_data) {
        oldAllocator = __context.Allocator;
        __context.Allocator = {__temporary_allocator, __temporary_allocator_data};
    }

    String_Builder builder;
    print_to_builder(builder, format, std::forward<Args>(argsPack)...);
    return to_string(builder);

    if (__temporary_allocator_data) {
        __context.Allocator = oldAllocator;
    }
}

// Prints formatted string to console
template <typename... Args>
inline void print(string const &format, Args &&... argsPack) {
    assert(__context.Log);
    __context.Log((tprint(format, std::forward<Args>(argsPack)...)));
}
