#include "guid.h"

#include "../internal/common.h"

LSTD_BEGIN_NAMESPACE

bool guid::operator==(const guid &other) const { return compare_memory(Data, other.Data, 16) == npos; }
bool guid::operator!=(const guid &other) const { return !(*this == other); }

LSTD_END_NAMESPACE
