#pragma once

//
// A header which imports common types, numeric info,
//     common math functions, definitions for the macros:
//	assert, defer, For, For_enumerate ...
//		static_for, range
// ... and others.
//
// And also memory stuff:
//     memcpy, memset, memset0, memcmp
//
// Really very common lightweight stuff that's used all the time.
//

#include "common/math.h" // also includes type_info.h, numeric.h, and ieee.h

#include "common/assert.h"
#include "common/debug_break.h"
#include "common/defer.h"
#include "common/enumerate.h"
#include "common/fmt.h"
#include "common/for.h"
#include "common/namespace.h"
#include "common/platform.h"
#include "common/range.h"

#include "common/cpp/arg.h"
#include "common/cpp/compare.h"
#include "common/cpp/initializer_list.h"
#include "common/cpp/source_location.h"

#include "context/context.h"
#include "memory/allocation.h"

//
// If we aren't building with CRT then:
//
// Cephes provides our replacement for the math functions found in virtually all
// standard libraries. Also provides functions for extended precision
// arithmetic, statistical functions, physics, astronomy, etc.
// https://www.netlib.org/cephes/
// Note: We don't include everything from it, just cmath for now.
//       Statistics is a thing we will most definitely include as well in the
//       future. Everything else you can include on your own in your project (we
//       don't want to be bloat-y).
//
// Note: Important difference,
// atan2's return range is 0 to 2PI, and not -PI to PI (as per normal in the C
// standard library).
//
// Parts of the source code that we modified are marked with :WEMODIFIEDCEPHES:
//
// @TODO: We should always have our own math functions
// because otherwise they'd differ from compiler to compiler.
// This is a horrendous mistake the C++ committee has allowed to happen.

/*
Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1995, 2000 by Stephen L. Moshier
*/
#include "third_party/cephes/maths_cephes.h"

//
// Some personal preferences:
//
// I prefer to type null over nullptr but they are exactly the same
using null_t = decltype(nullptr);
inline const null_t null = nullptr;

#ifndef NULL
#define NULL 0
#endif

// Semantics to avoid the use of & when the symbol is not used as a unary or
// binary operator.
//
// e.g.
//      void print_array_to_file(array<u8> no_copy bytes) { ... }
//      void modify_array(array<u8> ref bytes) { ... }
//
#define no_copy const &
#define ref &

// Used to mark functions for which the caller is supposed to free the result.
// This at leasts makes the compiler warn the caller if they've decided to
// discard the result.
//
// e.g.
//		mark_as_leak string make_string(...) { ... }
//
#define mark_as_leak [[nodiscard("Leak")]]

//
// Personal preference
// e.g. cast(int) 5.0
//
#define cast(x) (x)

// Helper macro for, e.g flag enums
//
// enum flags {
//	Flag_1 = BIT(0),
//  Flag_1 = BIT(1)
//  Flag_1 = BIT(2)
//  ...
// };
//
#define BIT(x) (1 << (x))

// Gives the offset of a member in a struct (in bytes)
#define offset_of(s, field) ((u64) & ((s *)(0))->field)

// Tau supremacy https://tauday.com/tau-manifesto
#define TAU 6.283185307179586476925286766559
#define PI (TAU / 2)

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

LSTD_BEGIN_NAMESPACE

template <typename T> void swap(T &a, T &b) {
  T c = a;
  a = b;
  b = c;
}

template <typename T, s64 N> void swap(T (&a)[N], T (&b)[N]) {
  For(range(N)) swap(a[it], b[it]);
}

/*
 * @Volatile with README.md
 * :TypePolicy:
 * - Keep it simple and data-oriented. Design data to simplify solutions and
minimize abstraction layers.
 * - Use `struct` instead of `class`, and keep everything public.
 * - Provide a default constructor that does minimal work.
 * - Avoid copy/move constructors and destructors.
 * - Never throw exceptions. Instead, return multiple values using structured
 * bindings (C++17). They make code complicated. When you can't handle an error and
 * need to exit from a function, return multiple values. 
 *     auto [content, success] = path_read_entire_file("data/hello.txt");
 * In general, error conditions (which require returning a status) should be
 * rare. The code should just do the correct stuff. I find that using exceptions
 * leads to this mentality of "giving up and passing the responsibility to handle
 * error cases to the caller". Howoever, that quickly becomes complicated and
 * confidence is lost on what could happen and where. Code in general likes to grow
 * in complexity combinatorially as more functionality is added, if we also give up
 * the linear structure of code by using exceptions then that's a disaster waiting
 * to happen.
 *
 * Example:
 * Arrays are basic wrappers around contiguous memory with three fields (`Data`,
`Count`, and `Allocated`).
 * By default, arrays are views. To make them dynamic, call `reserve(arr)` or
`make_array(...)`.
 * To allocate and free memory, call `reserve(arr)` and `free(arr)` or use
`defer(free(arr))`.
 *
 * `string`s behave like arrays but have different types to avoid conflicts.
 * They take indices to code points (as they are UTF-8 by default) and are not
null-terminated.
 * To make a deep copy, use `clone()`: `newPath = clone(path)`.
 * Functions accepting indices allow negative reversed indexing (Python-style)
for easy access to elements from the end.
 */

LSTD_END_NAMESPACE

// @Cleanup These implementations are not ideal
extern "C" {
inline void *memmove(void *dstpp, const void *srcpp, u64 len) {
  auto *dst = (byte *)dstpp;
  auto *src = (byte *)srcpp;
  if (len == 0)
    return dstpp;
  For(range(len - 1, -1, -1)) dst[it] = src[it];
  return dst;
}

inline void *memcpy(void *dstpp, const void *srcpp, u64 len) {
  if ((u64)dstpp > (u64)srcpp &&
      (s64)((byte *)dstpp - (byte *)srcpp) < (s64)len) {
    //
    // Careful. Buffers overlap. You should use memmove in this case.
    //
    // If this bug isn't caught until Release, then bad shit happens.
    // So in order to make it work nevertheless we do memmove.
    // I wish the C standard didn't make a distinction between the
    // two functions, but we're stuck with that.
    //
    // This makes calling memmove superfluous, and personally,
    // I'm ok with that.
    return memmove(dstpp, srcpp, len);
  } else {
    auto *dst = (byte *)dstpp;
    auto *src = (byte *)srcpp;
    while (len--)
      *dst++ = *src++;
  }
  return dstpp;
}

inline void *memset(void *dstpp, int c, u64 len) {
  u64 dstp = (u64)dstpp;

  if (len >= 8) {
    size_t xlen;
    u64 cccc;

    cccc = (byte)c;
    cccc |= cccc << 8;
    cccc |= cccc << 16;
    cccc |= (cccc << 16) << 16;

    /* There are at least some bytes to set.
No need to test for LEN == 0 in this alignment loop.  */
    while (dstp % 8 != 0) {
      ((byte *)dstp)[0] = c;
      dstp += 1;
      len -= 1;
    }

    /* Write 8 `op_t' per iteration until less than 8 `op_t' remain.  */
    xlen = len / (8 * 8);
    while (xlen > 0) {
      ((u64 *)dstp)[0] = cccc;
      ((u64 *)dstp)[1] = cccc;
      ((u64 *)dstp)[2] = cccc;
      ((u64 *)dstp)[3] = cccc;
      ((u64 *)dstp)[4] = cccc;
      ((u64 *)dstp)[5] = cccc;
      ((u64 *)dstp)[6] = cccc;
      ((u64 *)dstp)[7] = cccc;
      dstp += 8 * 8;
      xlen -= 1;
    }
    len %= 8 * 8;

    xlen = len / 8;
    while (xlen > 0) {
      ((u64 *)dstp)[0] = cccc;
      dstp += 8;
      xlen -= 1;
    }
    len %= 8;
  }

  while (len > 0) {
    ((byte *)dstp)[0] = c;
    dstp += 1;
    len -= 1;
  }

  return dstpp;
}

inline void *memset0(void *dst, u64 numInBytes) {
  return memset((char *)dst, (char)0, numInBytes);
}

inline int memcmp(const void *s1, const void *s2, u64 n) {
  auto *p1 = (byte *)s1;
  auto *p2 = (byte *)s2;
  For(range(n)) {
    if (p1[it] != p2[it])
      return p1[it] - p2[it];
  }
  return 0;
}
}