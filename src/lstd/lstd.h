#pragma once

#include "atomic.h"
#include "bits.h"
#include "common.h"
#include "context.h"
#include "math.h"
#include "memory.h"
#include "range.h"
#include "type_info.h"

LSTD_BEGIN_NAMESPACE

/*
 * @Volatile with README.md
 * :TypePolicy:
 * - Keep it simple and data-oriented. Design data to simplify solutions and
 * minimize abstraction layers.
 * - Use `struct` instead of `class`, and keep everything public.
 * - Provide a default constructor that does minimal work.
 * - Avoid copy/move constructors and destructors.
 * - Never throw exceptions. Instead, return multiple values using structured
 * bindings (C++17). They make code complicated. When you can't handle an error
 * and need to exit from a function, return multiple values.
 *     auto [content, success] = path_read_entire_file("data/hello.txt");
 * In general, error conditions (which require returning a status) should be
 * rare. The code should just do the correct stuff. I find that using exceptions
 * leads to this mentality of "giving up and passing the responsibility to
 * handle error cases to the caller". Howoever, that quickly becomes complicated
 * and confidence is lost on what could happen and where. Code in general likes
 * to grow in complexity combinatorially as more functionality is added, if we
 * also give up the linear structure of code by using exceptions then that's a
 * disaster waiting to happen.
 *
 * Example:
 * Arrays are basic wrappers around contiguous memory with three fields (`Data`,
 * `Count`, and `Allocated`). By default, arrays are views. To make them
 * dynamic, call `reserve(arr)` or `make_array(...)`. To allocate and free
 * memory, call `reserve(arr)` and `free(arr)` or use `defer(free(arr))`.
 *
 * `string`s behave like arrays but have different types to avoid conflicts.
 * They take indices to code points (as they are UTF-8 by default) and are not
 * null-terminated. To make a deep copy, use `clone()`: `newPath = clone(path)`.
 * Functions accepting indices allow negative reversed indexing (Python-style)
 * for easy access to elements from the end.
 */

template <typename T> void swap(T &a, T &b) {
  T c = a;
  a = b;
  b = c;
}

template <typename T, s64 N> void swap(T (&a)[N], T (&b)[N]) {
  For(range(N)) swap(a[it], b[it]);
}

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
