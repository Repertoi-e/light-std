module;

#include "../memory/string_builder.h"

export module fmt.format_float.fallback;

import fmt.format_float.specs;

import lstd.big_integer;

//
// Formats value using a variation of the Fixed-Precision Positive
// Floating-Point Printout ((FPP)^2) algorithm by Steele & White:
// https://fmt.dev/papers/p372-steele.pdf.
//

LSTD_BEGIN_NAMESPACE

// export s32 steele_white_format_float(string_builder &builder, f64 value, s32 exp, s32 precision, bool binary32) {
// }

LSTD_END_NAMESPACE
