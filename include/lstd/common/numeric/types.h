#pragma once

#include "../platform.h"

//
// The following integral types are defined here:
//      s8, s16, s32, s64, s128,
//      ... and corresponding unsigned types: u8, u16, u32, u64, u128,
//      vector types (aligned on 16 byte boundaries for SIMDs):
//          u8v16, u16v8, u32v4, u64v2, s8v16, s16v8, s32v4, s64v2, f32v4, f64v2
//		f32 (float), f64 (double), wchar (for Windows),
//      code_point (for the integer value of a Unicode code point),
//      and byte (unsigned char)
//
// Note: We don't support long doubles (lf64) or operations with them throughout
// the library.
//

//
// Fundamental types:
//
using s8 = char;
using s16 = short;
using s32 = int;
using s64 = long long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned;
using u64 = unsigned long long;

using wchar = wchar_t;  // Only useful for Windows calls. Please don't use
                        // utf-16 in your programs...

using code_point =
    char32_t;  // Holds the integer value of a Unicode code point.

using byte = unsigned char;

using f32 = float;
using f64 = double;

#if !defined(_SIZE_T)
#if BITS == 32
using size_t = u32;
using ptrdiff_t = s32;
using intptr_t = s32;
using time_t = s32;
#else
using size_t = u64;
using ptrdiff_t = s64;
using intptr_t = s64;
using time_t = s64;
#endif
#endif

using usize = size_t;

//
// Vector types (aligned on 16 byte boundaries for SIMDs)
//
template <typename T, s64 Count>
union alignas(16) base_vector_type {
  T Values[Count];
};

using u8v16 = base_vector_type<u8, 16>;
using u16v8 = base_vector_type<u16, 8>;
using u32v4 = base_vector_type<u32, 4>;
using u64v2 = base_vector_type<u64, 2>;

using s8v16 = base_vector_type<s8, 16>;
using s16v8 = base_vector_type<s16, 8>;
using s32v4 = base_vector_type<s32, 4>;
using s64v2 = base_vector_type<s64, 2>;
using f32v4 = base_vector_type<f32, 4>;
using f64v2 = base_vector_type<f64, 2>;

//
// Convenience storage literal operators, allows for specifying sizes like this:
//  s64 a = 10_MiB;
//  s64 a = 20_billion;
//
// The International Electronic Commission established the term kibibyte
// for 1024 bytes, because the metric system already has a use for the
// prefix "kilo" meaning a thousand. So 1 KB = 1000 bytes.
//
// In practice, however, when buying storage or downloading files or
// looking in Windows explorer, KB has the meaning of 1024 bytes.
// Suddenly switching to KiB for no particular reason (except having
// the feeling of being correct or superior that you know a term
// which others don't) would cause confusion to users.
//
// However, we are programmers. And in general the decision of the IEC
// does sound logical. So for the sake of being exact we will name
// these literals with the proper term.
//
//
// _B is for completeness, really useless though
constexpr u64 operator"" _B(u64 i) { return i; }
constexpr u64 operator"" _KiB(u64 i) { return i << 10; }
constexpr u64 operator"" _MiB(u64 i) { return i << 20; }
constexpr u64 operator"" _GiB(u64 i) { return i << 30; }

constexpr u64 operator"" _thousand(u64 i) { return i * 1000; }
constexpr u64 operator"" _million(u64 i) { return i * 1000000; }
constexpr u64 operator"" _billion(u64 i) { return i * 1000000000; }
