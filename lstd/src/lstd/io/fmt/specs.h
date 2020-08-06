#pragma once

#include "error_handler.h"
#include "text_style.h"
#include "value.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

// The optional align is one of the following:
//   '<' - Forces the field to be left-aligned within the available space (default)
//   '>' - Forces the field to be right-aligned within the available space.
//   '=' - Forces the padding to be placed after the sign (if any)
//         but before the digits.  This is used for printing fields
//         in the form '+000000120'. This alignment option is only
//         valid for numeric types.
//   '^' - Forces the field to be centered within the available space
enum class alignment { NONE = 0,
                       LEFT,     // <
                       RIGHT,    // >
                       NUMERIC,  // =
                       CENTER    // ^
};

// The 'sign' option is only valid for numeric types, and can be one of the following:
//   '+'  - Indicates that a sign should be used for both positive as well as negative numbers
//   '-'  - Indicates that a sign should be used only for negative numbers (default)
//   ' '  - Indicates that a leading space should be used on positive numbers
enum class sign {
    NONE = 0,
    PLUS,

    // MINUS has the same behaviour as NONE on our types,
    // but the user might want to have different formating
    // on their custom types when minus is specified,
    // so we record it when parsing anyway.
    MINUS,
    SPACE,
};

struct format_specs {
    char32_t Fill = ' ';
    alignment Align = alignment::NONE;

    sign Sign = sign::NONE;
    bool Hash = false;

    u32 Width = 0;
    s32 Precision = -1;

    char Type = 0;
};

// Refers to an argument by index or name.
// Used for dynamic arguments (specifying width/precision not as a constant in the format string but as a separate argument).
struct arg_ref {
    enum class kind { NONE = 0,
                      INDEX,
                      NAME };

    kind Kind = kind::NONE;
    union {
        u32 Index;
        string Name;
    };

    arg_ref() : Index(0) {}
    arg_ref(u32 index) : Kind(kind::INDEX), Index(index) {}
    arg_ref(const string &name) : Kind(kind::NAME), Name(name) {}

    arg_ref &operator=(u32 index) {
        Kind = kind::INDEX;
        Index = index;
        return *this;
    }
};

// Dynamic means that the width/precision was specified in a separate argument and not as a constant in the format string
struct dynamic_format_specs : format_specs {
    arg_ref WidthRef;
    arg_ref PrecisionRef;
};
}  // namespace fmt

LSTD_END_NAMESPACE
