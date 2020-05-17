#pragma once

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Useful for generating unique ids.
// Guaranteed to generate a unique id (time-based) - as long as all ids are generated using this object.
struct guid {
    char Data[16]{};

    guid() = default;  // By default the guid is zero
    explicit guid(array_view<char> data) { copy_memory(Data, data.begin(), 16); }

    bool is_zero() { return guid() == *this; }

    bool operator==(const guid &other) const;
    bool operator!=(const guid &other) const;
};

guid new_guid();

LSTD_END_NAMESPACE
