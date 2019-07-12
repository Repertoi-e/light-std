#pragma once

#include "error_handler.h"
#include "value.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

enum class alignment { DEFAULT = 0, LEFT, RIGHT, CENTER, NUMERIC };
enum class flag : u32 { SIGN = BIT(0), PLUS = BIT(1), MINUS = BIT(2), HASH = BIT(3) };

constexpr flag operator|(flag lhs, flag rhs) {
    using T = underlying_type_t<flag>;
    return (flag)((T) lhs | (T) rhs);
}

constexpr flag &operator|=(flag &lhs, flag rhs) {
    using T = underlying_type_t<flag>;
    lhs = (flag)((T) lhs | (T) rhs);
    return lhs;
}

struct align_specs {
    u32 Width;
    char32_t Fill;
    alignment Align;

    constexpr align_specs(u32 width, char32_t fill, alignment align = alignment::DEFAULT)
        : Width(width), Fill(fill), Align(align) {}
};

struct format_specs : public align_specs {
    flag Flags = (flag) 0;
    s32 Precision = -1;
    byte Type;
	color Color;

    constexpr format_specs(u32 width = 0, byte type = 0, char32_t fill = ' ')
        : align_specs(width, fill), Type(type) {}

    constexpr bool has_flag(flag flag) const { return ((u32) Flags & (u32) flag) != 0; }
};

struct string_view_metadata {
    size_t Offset = 0;
    size_t Size = 0;

    constexpr string_view_metadata() = default;

    constexpr string_view_metadata(string_view primary, string_view view)
        : Offset(view.Data - primary.Data), Size(view.ByteLength) {}
    constexpr string_view_metadata(size_t offset, size_t size) : Offset(offset), Size(size) {}

    constexpr string_view to_view(const byte *str) const { return {str + Offset, Size}; }
};

struct arg_ref {
    enum class kind { NONE = 0, INDEX, NAME };
    kind Kind = kind::NONE;
    union {
        u32 Index;
        string_view_metadata Name;
    };

    constexpr arg_ref() : Index(0) {}
    constexpr explicit arg_ref(u32 index) : Kind(kind::INDEX), Index(index) {}
    constexpr explicit arg_ref(string_view_metadata name) : Kind(kind::NAME), Name(name) {}

    constexpr arg_ref &operator=(u32 index) {
        Kind = kind::INDEX;
        Index = index;
        return *this;
    }
};

struct dynamic_format_specs : format_specs {
    arg_ref WidthRef;
    arg_ref PrecisionRef;
};

namespace internal {

template <typename Handler>
constexpr void handle_int_type_spec(char32_t spec, Handler *handler) {
    switch (spec) {
        case 0:
        case 'd':
            handler->on_dec();
            break;
        case 'x':
        case 'X':
            handler->on_hex();
            break;
        case 'b':
        case 'B':
            handler->on_bin();
            break;
        case 'o':
            handler->on_oct();
            break;
        case 'n':
            handler->on_num();
            break;
        case 'c':
            handler->on_char();
            break;
        default:
            handler->on_error();
    }
}

template <typename Handler>
constexpr void handle_float_type_spec(char32_t spec, Handler *handler) {
    switch (spec) {
        case 0:
        case 'g':
        case 'G':
            handler->on_general();
            break;
        case 'e':
        case 'E':
            handler->on_exp();
            break;
        case 'f':
        case 'F':
            handler->on_fixed();
            break;
        case '%':
            handler->on_percent();
            break;
        case 'a':
        case 'A':
            handler->on_hex();
            break;
        default:
            handler->on_error();
            break;
    }
}

template <typename Handler>
constexpr void handle_cstring_type_spec(char32_t spec, Handler *handler) {
    if (spec == 0 || spec == 's') {
        handler->on_string();
    } else if (spec == 'p') {
        handler->on_pointer();
    } else {
        handler->on_error();
    }
}

struct int_type_checker {
    error_handler_t ErrorHandlerFunc;
    error_context ErrorContext;

    constexpr void on_dec() {}
    constexpr void on_hex() {}
    constexpr void on_bin() {}
    constexpr void on_oct() {}
    constexpr void on_num() {}
    constexpr void on_char() {}

    constexpr void on_error() { ErrorHandlerFunc("Invalid type specifier", ErrorContext); }
};

struct float_type_checker {
    error_handler_t ErrorHandlerFunc;
    error_context ErrorContext;

    constexpr void on_general() {}
    constexpr void on_exp() {}
    constexpr void on_fixed() {}
    constexpr void on_percent() {}
    constexpr void on_hex() {}

    constexpr void on_error() { ErrorHandlerFunc("Invalid type specifier", ErrorContext); }
};

struct cstring_type_checker {
    error_handler_t ErrorHandlerFunc;
    error_context ErrorContext;

    constexpr void on_string() {}
    constexpr void on_pointer() {}

    constexpr void on_error() { ErrorHandlerFunc("Invalid type specifier", ErrorContext); }
};
}  // namespace internal

}  // namespace fmt

LSTD_END_NAMESPACE

// :ExplicitDeclareIsPod
DECLARE_IS_POD(fmt::arg_ref, true)
DECLARE_IS_POD(fmt::format_specs, true)
