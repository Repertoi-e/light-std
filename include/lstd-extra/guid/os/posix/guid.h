#pragma once

#include "../guid_common.h"
#include "lstd/stack_array.h"

#include <uuid/uuid.h>

LSTD_BEGIN_NAMESPACE

// Guaranteed to generate a unique id each time (time-based)
guid create_guid() {
  uuid_t uuid;
  uuid_generate_time(uuid);

  guid result;
  memcpy(result.Data, uuid, sizeof(result.Data));
  return result;
}

LSTD_END_NAMESPACE
