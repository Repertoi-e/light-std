#pragma once

#include "array_like.h"

LSTD_BEGIN_NAMESPACE

// Used for generating unique ids
struct guid {
    // :CodeReusability: Make this object array-like so we can use compare, compare_lexicographically, ==, !=, < operators, etc.. for free!
    static constexpr bool IS_ARRAY_LIKE = true;

    byte Data[16]{};
    static constexpr s64 Count = 16;

    constexpr guid() {}  // By default the guid is zero

    constexpr guid(const initializer_list<byte> &data) {
        assert(data.size() >= 16);
        copy_memory(Data, data.begin(), 16);
    }

    constexpr explicit guid(bytes data) {
        assert(data.Count >= 16);
        copy_memory(Data, data.Data, 16);
    }

    constexpr operator bool() { return (guid{} != *this); }
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
