#pragma once

#include "../internal/common.h"
#include "hasher.h"
#include "string.h"
#include "guid.h"

//
// !!! THESE ARE NOT SUPPOSED TO BE CRYPTOGRAPHICALLY SECURE !!!
//
// This header provides a way to hash types.
// If you want to implement a custom hash function for a type,
// implement it in global namespace like this:
//
// LSTD_BEGIN_NAMESPACE
// uptr_t get_hash(T value) { return ...; }
// LSTD_END_NAMESPACE
//

LSTD_BEGIN_NAMESPACE

// Floats are handled in the general hash function here.
// The output depends on the endianness of the machine,
// because we are reinterpreting the float's bits as unsinged numbers
template <typename T>
constexpr uptr_t get_hash(const T &value) {
    hasher h(0);
    h.add((const char *) &value, sizeof(T));
    return h.hash();
}

// Partial specialization for pointers
template <typename T>
constexpr enable_if_t<is_pointer_v<T>, uptr_t> get_hash(const T value) {
    return (uptr_t) value;
}

// Partial specialization for arrays of known size
template <typename T>
constexpr enable_if_t<is_array_v<T> && is_array_of_known_bounds_v<T>, uptr_t> get_hash(const T value) {
    hasher h(0);
    h.add((const char *) value, sizeof(remove_extent_t<T>) * extent_v<T>);
    return h.hash();
}

// Hashes for integer types
#define TRIVIAL_HASH(T) \
    constexpr uptr_t get_hash(T value) { return (uptr_t) value; }

TRIVIAL_HASH(s8);
TRIVIAL_HASH(u8);

TRIVIAL_HASH(s16);
TRIVIAL_HASH(u16);

TRIVIAL_HASH(s32);
TRIVIAL_HASH(u32);

TRIVIAL_HASH(s64);
TRIVIAL_HASH(u64);

TRIVIAL_HASH(bool);

#undef TRIVIAL_HASH

// Hash for strings
constexpr uptr_t get_hash(string_view value) {
    hasher h(0);
    h.add(value.Data, value.ByteLength);
    return h.hash();
}

inline uptr_t get_hash(string value) {
    uptr_t hash = 5381;
    For(value) hash = ((hash << 5) + hash) + it;
    return hash;
}

// Hash for guid
inline uptr_t get_hash(guid value) {
    uptr_t hash = 5381;
    For(value.Data) hash = ((hash << 5) + hash) + it;
    return hash;
}


LSTD_END_NAMESPACE
