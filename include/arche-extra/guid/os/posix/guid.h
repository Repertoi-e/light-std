#pragma once

#include "../guid_common.h"
#include "arche/stack_array.h"

#include <uuid/uuid.h>

ARCHE_BEGIN_NAMESPACE

// Guaranteed to generate a unique id each time (time-based)
guid create_guid() {
    uuid_t uuid;
    uuid_generate_time(uuid);

    guid result;
    memcpy(result.Data, uuid, sizeof(result.Data));
    return result;
}

ARCHE_END_NAMESPACE
