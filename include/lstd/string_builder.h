#pragma once

#include "xar.h"
#include "string.h"
#include "writer.h"

LSTD_BEGIN_NAMESPACE

//
// String builder is good for building large strings without having to
// constantly reallocate. Starts with a base-sized stack chunk. When that fills
// up it exponentially allocates buffers, each twice the size.
//
// This is using exponential array internally, so look at "xar.h"
//
using string_builder = exponential_array<char, 25, 10, true>; // base chunk 1 << 10 (1 KiB) on stack; 25 chunks total

template <typename>
const bool is_string_builder = false;
template <usize N, usize BASE_SHIFT>
const bool is_string_builder<exponential_array<char, N, BASE_SHIFT>> = true;
template <usize N, usize BASE_SHIFT, bool STACK_FIRST>
const bool is_string_builder<exponential_array<char, N, BASE_SHIFT, STACK_FIRST>> = true;

template <typename T>
concept any_string_builder = is_string_builder<T>;

inline void add(any_string_builder auto ref builder, const char *data, s64 size, allocator alloc = {}) {
  if (size <= 0) return;

  // Ensure capacity for the entire append in one go. No-ops for already allocated chunks.
  reserve(builder, builder.Count + (usize)size, alloc);

  s64 i = 0;
  const usize base_size = 1u << builder.BASE_SHIFT;
  while (i < size) {
    usize chunk_idx = 0;
    if (builder.Count < base_size) {
      chunk_idx = 0;
    } else if (builder.Count < (base_size << 1)) {
      chunk_idx = 1;
    } else {
      // For i >= 2, start(i) = base * 2^(i-1); choose i so that start(i) <= Count < start(i)+size(i)
      usize units = builder.Count >> builder.BASE_SHIFT; // Count in base units
      chunk_idx = (usize)msb(units) + 1; // moves to next chunk at exact boundaries
    }

    const usize chunk_size = (chunk_idx <= 1) ? base_size : (base_size << (chunk_idx - 1));
    const usize chunk_start = (chunk_idx == 0) ? 0 : (chunk_idx == 1) ? base_size : (base_size << (chunk_idx - 1));

    usize offset_in_chunk = builder.Count - chunk_start;

    // How many bytes we can copy into this chunk
    usize space_left = chunk_size - offset_in_chunk;
    assert(space_left > 0);
    usize to_copy = min(space_left, (usize)(size - i));

    char *dst = builder.get_chunk_ptr(chunk_idx) + offset_in_chunk;
    memcpy(dst, data + i, to_copy);
    builder.Count += to_copy;
    i += to_copy;
  }
}


inline void add(any_string_builder auto ref builder, code_point cp) {
  char encodedCp[4];
  utf8_encode_cp(encodedCp, cp);
  add(builder, encodedCp, utf8_get_size_of_cp(encodedCp));
}

inline void add(any_string_builder auto ref builder, string str) {
    add(builder, str.Data, str.Count);
}

inline string builder_to_string(any_string_builder auto ref builder, allocator alloc = {}) {
    string result;
  reserve(result, builder.Count, alloc);

    usize copied = 0;
  exponential_array_visit_chunks(builder, [&](const char *chunk_data, usize chunk_size, usize /*chunk_index*/) {
    memcpy(result.Data + copied, chunk_data, chunk_size);
    copied += chunk_size;
    return true; // Continue iteration
  });
    result.Count = copied;
    return result;
}

inline string builder_to_string_and_clear(any_string_builder auto ref builder, allocator alloc = {}) {
    string result = builder_to_string(builder, alloc);
  builder.Count = 0;
    return result;
}

inline string builder_to_string_and_free(any_string_builder auto ref builder, allocator alloc = {}) {
    string result = builder_to_string(builder, alloc);
    free(builder);
    return result;
}

inline bool utf8_normalize_nfc_to_string_builder(const char *str, s64 byteLength, any_string_builder auto ref builder)
{
  if (!str || byteLength < 0)
    return false;

  const char *p = str;
  const char *end = str + byteLength;
  stack_array<code_point, 1024> segBuf;

  while (p < end)
  {
    s64 segN = 0;
    if (!utf8_segment_nfd(p, end, segBuf, segN))
      return false;

    // Canonical composition
    stack_array<code_point, 1024> compBuf;
    s64 compN = 0;
    if (segN > 0)
    {
      compBuf.Data[compN++] = segBuf.Data[0];
      s64 starterPos = 0;
      u8 lastCC = 0;
      for (s64 i = 1; i < segN; ++i)
      {
        code_point c = segBuf.Data[i];
        u8 cc = unicode_combining_class(c);
        code_point starter = compBuf.Data[starterPos];
        code_point m = unicode_compose_pair(starter, c);
        if (m && (lastCC < cc))
        {
          compBuf.Data[starterPos] = m;
        }
        else
        {
          compBuf.Data[compN++] = c;
          if (cc == 0)
          {
            starterPos = compN - 1;
            lastCC = 0;
          }
          else
          {
            lastCC = cc;
          }
        }
      }
    }

    // Emit composed segment into string_builder
    for (s64 i = 0; i < compN; ++i)
    {
      add(builder, compBuf.Data[i]);
    }
  }

  return true;
}

inline bool utf8_normalize_nfd_to_string_builder(const char *str, s64 byteLength, any_string_builder auto ref builder)
{
  if (!str || byteLength < 0)
    return false;

  const char *p = str;
  const char *end = str + byteLength;
  stack_array<code_point, 1024> segBuf;

  while (p < end)
  {
    s64 segN = 0;
    if (!utf8_segment_nfd(p, end, segBuf, segN))
      return false;

    // Emit NFD directly into string_builder
    for (s64 i = 0; i < segN; ++i)
    {
      add(builder, segBuf.Data[i]);
    }
  }

  return true;
}

// Makes a normalized copy (NFC) of a string. Returns a new owning string.
// If input is invalid UTF-8, returns a clone of the original.
mark_as_leak inline string make_string_normalized_nfc(string s)
{
  if (!s.Data || s.Count == 0)
    return {};

  string_builder out;
  defer(free(out));

  // Reserve buckets at least for the original size, 
  // this should be enough space with at most 1 more 
  // allocation since next bucket is double the previous.
  reserve(out, s.Count); 

  bool ok = utf8_normalize_nfc_to_string_builder(s.Data, s.Count, out);
  if (!ok) return {};

  return builder_to_string(out);
}

//
// A writer to output to a string
//
struct string_builder_writer : writer {
  string_builder *Builder;

  void write(const char *data, s64 count) override {
    add(*Builder, data, count);
  }
  void flush() override {}
};

LSTD_END_NAMESPACE
