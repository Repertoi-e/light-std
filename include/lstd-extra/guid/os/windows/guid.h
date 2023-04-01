#pragma once

#include "../guid_common.h"
#include "lstd/os/windows/api.h"
#include "lstd/stack_array.h"

LSTD_BEGIN_NAMESPACE

// Guaranteed to generate a unique id each time (time-based)
guid create_guid() {
  GUID g;
  CoCreateGuid(&g);

  auto data =
      make_stack_array((u8)(g.Data1 >> 24 & 0xFF), (u8)(g.Data1 >> 16 & 0xFF),
                       (u8)(g.Data1 >> 8 & 0xFF), (u8)(g.Data1 & 0xff),

                       (u8)(g.Data2 >> 8 & 0xFF), (u8)(g.Data2 & 0xff),

                       (u8)(g.Data3 >> 8 & 0xFF), (u8)(g.Data3 & 0xFF),

                       g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3],
                       g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);

  guid result;
  For(range(16)) result.Data[it] = data[it];
  return result;
}

LSTD_END_NAMESPACE
