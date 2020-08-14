#include "guid.h"

#include "../internal/common.h"

LSTD_BEGIN_NAMESPACE

s64 guid::compare(const guid &other) const { return compare_memory(Data, other.Data, 16); }

LSTD_END_NAMESPACE
