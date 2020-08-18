#pragma once

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Used for generating unique ids
struct guid {
    char Data[16]{};

    constexpr guid() {}  // By default the guid is zero

    constexpr guid(const initializer_list<u8> &data) {
        assert(data.size() >= 16);
        copy_memory(Data, data.begin(), 16);
    }

    constexpr explicit guid(const array<char> &data) {
        assert(data.Count >= 16);
        copy_memory(Data, data.Data, 16);
    }

    bool is_zero() { return guid() == *this; }

    s64 compare(const guid &other) const;

    bool operator==(const guid &other) const { return compare(other) == -1; }
    bool operator!=(const guid &other) const { return !(*this == other); }
};

// Guaranteed to generate a unique id each time (time-based)
guid guid_new();

LSTD_END_NAMESPACE
