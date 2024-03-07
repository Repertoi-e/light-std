#pragma once

#include "float_dragonbox.h"
#include "float_grisu.h"
#include "specs.h"

LSTD_BEGIN_NAMESPACE

inline void add_u64(string_builder *builder, u64 value) {
  const s32 BUFFER_SIZE = numeric<u64>::digits10;
  char buffer[BUFFER_SIZE];

  auto *p = buffer + BUFFER_SIZE - 1;

  if (!value) {
    *p-- = '0';
  }

  while (value) {
    auto d = value % 10;
    *p-- = (char)('0' + d);
    value /= 10;
  }

  ++p;  // Roll back
  add(builder, p, buffer + BUFFER_SIZE - p);
}

// The returned exponent is the exponent base 10 of the LAST written digit in
// _floatBuffer_. In the end, _floatBuffer_ contains the digits of the final
// number to be written out, without the dot.
s32 fmt_format_non_negative_float(string_builder *floatBuffer,
                                  is_floating_point auto value, s32 precision,
                                  fmt_float_specs no_copy specs) {
  assert(value >= 0);

  bool fixed = specs.Format == fmt_float_specs::FIXED;
  if (value == 0) {
    if (precision <= 0 || !fixed) {
      add(floatBuffer, U'0');
      return 0;
    }

    // @Speed
    For(range(precision)) { add(floatBuffer, U'0'); }
    return -precision;
  }

  // If precision is still -1 (because no precision and no spec type was
  // specified), use Dragonbox for the shortest format. We set a default
  // precision to 6 before calling this routine in the cases when the format
  // string didn't specify a precision, but specified a specific format
  // (GENERAL, EXP, FIXED, etc.)
  if (precision < 0) {
    auto dec = dragonbox_format_float(value);
    add_u64(floatBuffer, dec.Significand);
    return dec.Exponent;
  }

  return grisu_format_float(floatBuffer, value, precision, specs);
}

LSTD_END_NAMESPACE
