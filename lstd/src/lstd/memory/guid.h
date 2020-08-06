#pragma once

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Useful for generating unique ids.
// Guaranteed to generate a unique id (time-based) - as long as all ids are generated using this object.
struct guid {
    char Data[16]{};

    constexpr guid() = default;  // By default the guid is zero
    constexpr explicit guid(const array<char> &data) {
        assert(data.Count >= 16);
        copy_memory(Data, data.Data, 16);
    }

    constexpr bool is_zero() { return guid() == *this; }

    constexpr bool operator==(const guid &other) const;
    constexpr bool operator!=(const guid &other) const;
};

guid new_guid();

LSTD_END_NAMESPACE
