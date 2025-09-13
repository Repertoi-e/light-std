#pragma once

#include "bits.h"
#include "common.h"
#include "string.h"

//
// !!! THESE ARE NOT SUPPOSED TO BE CRYPTOGRAPHICALLY SECURE !!!
//
// This header provides a way to hash any type.
// If you want to implement a custom hash function for your type,
// implement it in global namespace like this:
//
// LSTD_BEGIN_NAMESPACE
// u64 get_hash(T value) { return ...; }
// LSTD_END_NAMESPACE
//

LSTD_BEGIN_NAMESPACE

// xxhash64 without UB unaligned accesses:
// https://github.com/demetri/scribbles/blob/master/hashing/ub_aware_hash_functions.c
inline u64 get_hash_xxhash64(const byte *key, s64 len, u64 hash_so_far = 0)
{
  if (!key || len <= 0)
    return 0;

  // primes used in mul-rot updates
  u64 p1 = 0x9e3779b185ebca87, p2 = 0xc2b2ae3d27d4eb4f,
      p3 = 0x165667b19e3779f9, p4 = 0x85ebca77c2b2ae63, p5 = 0x27d4eb2f165667c5;

  // inital 32-byte (4x8) wide hash state
  const u64 h = hash_so_far;
  u64 s[4] = {h + p1 + p2, h + p2, h, h - p1};

  // bulk work: process all 32 byte blocks
  for (int i = 0; i < (len / 32); i++)
  {
    u64 b[4];
    memcpy(b, key + 4 * i, sizeof(b));

    for (int j = 0; j < 4; j++)
      b[j] = b[j] * p2 + s[j];
    for (int j = 0; j < 4; j++)
      s[j] = ((b[j] << 31) | (b[j] >> 33)) * p1;
  }

  // mix 32-byte state down to 8-byte state, initalize to value for short keys
  u64 s64 = (s[2] + p5);
  if (len >= 32)
  {
    s64 = ((s[0] << 1) | (s[0] >> 63)) + ((s[1] << 7) | (s[1] >> 57)) +
          ((s[2] << 12) | (s[2] >> 52)) + ((s[3] << 18) | (s[3] >> 46));
    for (int i = 0; i < 4; i++)
    {
      u64 ps = (((s[i] * p2) << 31) | ((s[i] * p2) >> 33)) * p1;
      s64 = (s64 ^ ps) * p1 + p4;
    }
  }
  s64 += len;

  // up to 31 bytes remain, process 0-3 8 byte blocks
  byte *tail = (byte *)(key + (len / 32) * 32);
  for (int i = 0; i < (len & 31) / 8; i++, tail += 8)
  {
    u64 b;
    memcpy(&b, tail, sizeof(u64));

    b *= p2;
    b = (((b << 31) | (b >> 33)) * p1) ^ s64;
    s64 = ((b << 27) | (b >> 37)) * p1 + p4;
  }

  // up to 7 bytes remain, process 0-1 4 byte block
  for (int i = 0; i < (len & 7) / 4; i++, tail += 4)
  {
    u64 b;
    memcpy(&b, tail, sizeof(b));

    b = (s64 ^ b) * p1;
    s64 = ((b << 23) | (b >> 41)) * p2 + p3;
  }

  // up to 3 bytes remain, process 0-3 1 byte blocks
  for (int i = 0; i < (len & 3); i++, tail++)
  {
    u64 b = s64 ^ (*tail) * p5;
    s64 = ((b << 11) | (b >> 53)) * p1;
  }

  // finalization mix
  s64 = (s64 ^ (s64 >> 33)) * p2;
  s64 = (s64 ^ (s64 >> 29)) * p3;
  return (s64 ^ (s64 >> 32));
}

// murmur3 32-bit without UB unaligned accesses
// https://github.com/demetri/scribbles/blob/master/hashing/ub_aware_hash_functions.c
inline u32 get_hash_murmur_32(const byte *key, s64 len, u32 hash_so_far = 0)
{
  u32 h = hash_so_far;

  // main body, work on 32-bit blocks at a time
  for (int i = 0; i < len / 4; i++)
  {
    u32 k;
    memcpy(&k, &key[i * 4], sizeof(k));

    k *= 0xcc9e2d51;
    k = ((k << 15) | (k >> 17)) * 0x1b873593;
    h = (((h ^ k) << 13) | ((h ^ k) >> 19)) * 5 + 0xe6546b64;
  }

  // load/mix up to 3 remaining tail bytes into a tail block
  u32 t = 0;
  byte *tail = ((byte *)key) + 4 * (len / 4);
  switch (len & 3)
  {
  case 3:
    t ^= tail[2] << 16;
  case 2:
    t ^= tail[1] << 8;
  case 1:
  {
    t ^= tail[0] << 0;
    h ^= ((0xcc9e2d51 * t << 15) | (0xcc9e2d51 * t >> 17)) * 0x1b873593;
  }
  }

  // finalization mix, including key length
  h = ((h ^ len) ^ ((h ^ len) >> 16)) * 0x85ebca6b;
  h = (h ^ (h >> 13)) * 0xc2b2ae35;
  return h ^ (h >> 16);
}

// Good enough hash for arrays of any type
// Note: If u have short low-entropy arrays, murmur might be better
inline u64 get_hash(any_array_like auto ref array)
{
  return get_hash_xxhash64(array.Data, array.Count * sizeof(array[0]));
}

// Hashes for integer types
#define TRIVIAL_HASH(T) \
  inline u64 get_hash(T value) { return (u64)value; }

TRIVIAL_HASH(s8);
TRIVIAL_HASH(u8);

TRIVIAL_HASH(s16);
TRIVIAL_HASH(u16);

TRIVIAL_HASH(s32);
TRIVIAL_HASH(u32);

TRIVIAL_HASH(s64);
TRIVIAL_HASH(u64);

TRIVIAL_HASH(bool);

// Hashing strings, based on murmur
// For larger strings, xxhash64 might be better
inline u64 get_hash(string value)
{
  return get_hash_murmur_32((const byte *) value.Data, value.Count, 0);
}

// Partial specialization for pointers
inline u64 get_hash(is_pointer auto value) { return (u64)value; }

LSTD_END_NAMESPACE
