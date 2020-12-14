#pragma once

#include "../types/compare.h"
#include "stack_array.h"

LSTD_BEGIN_NAMESPACE

// Used for generating unique ids
struct guid {
    stack_array<byte, 16> Data;

    constexpr guid() {}  // By default the guid is zero

    constexpr explicit guid(bytes data) {
        assert(data.Count >= 16);
        copy_memory(&Data[0], &data[0], 16);
    }

    constexpr operator bool() { return (guid{} != *this); }

    constexpr auto operator<=>(const guid &) const = default;
};

// Hash for guid
inline u64 get_hash(guid value) {
    u64 hash = 5381;
    For(value.Data) hash = ((hash << 5) + hash) + it;
    return hash;
}

// Guaranteed to generate a unique id each time (time-based)
guid guid_new();

LSTD_END_NAMESPACE
