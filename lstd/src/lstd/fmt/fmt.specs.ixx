module;

#include "../memory/string.h"
#include "../memory/string_utils.h"

export module fmt.specs;

LSTD_BEGIN_NAMESPACE

export {
    // The optional align is one of the following:
    //   '<' - Forces the field to be left-aligned within the available space (default)
    //   '>' - Forces the field to be right-aligned within the available space.
    //   '=' - Forces the padding to be placed after the sign (if any)
    //         but before the digits.  This is used for printing fields
    //         in the form '+000000120'. This alignment option is only
    //         valid for numeric types.
    //   '^' - Forces the field to be centered within the available space
    enum class fmt_alignment { NONE = 0,
                               LEFT,     // <
                               RIGHT,    // >
                               NUMERIC,  // =
                               CENTER    // ^
    };

    // The 'sign' option is only valid for numeric types, and can be one of the following:
    //   '+'  - Indicates that a sign should be used for both positive as well as negative numbers
    //   '-'  - Indicates that a sign should be used only for negative numbers (default)
    //   ' '  - Indicates that a leading space should be used on positive numbers
    enum class fmt_sign {
        NONE = 0,
        PLUS,

        // MINUS has the same behaviour as NONE on our types,
        // but the user might want to have different formating
        // on their custom types when minus is specified,
        // so we record it when parsing anyway.
        MINUS,
        SPACE,
    };

    struct fmt_specs {
        utf32 Fill = ' ';
        fmt_alignment Align = fmt_alignment::NONE;

        fmt_sign Sign = fmt_sign::NONE;
        bool Hash = false;

        u32 Width = 0;
        s32 Precision = -1;

        char Type = 0;
    };

    // Dynamic means that the width/precision was specified in a separate argument and not as a constant in the format string
    struct fmt_dynamic_specs : fmt_specs {
        s64 WidthIndex = -1;
        s64 PrecisionIndex = -1;
    };
}

LSTD_END_NAMESPACE
