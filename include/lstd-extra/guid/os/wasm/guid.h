#pragma once

#include "../guid_common.h"
#include "lstd/stack_array.h"
#include "lstd/random.h"

LSTD_BEGIN_NAMESPACE

guid create_guid() {
    guid result;
    
    // Generate random bytes
    u64 r1 = next_random();
    u64 r2 = next_random();
    
    // Copy random data
    memcpy(result.Data, &r1, 8);
    memcpy(result.Data + 8, &r2, 8);
    
    // Set UUID version 4 (random) in the version field (bits 12-15 of time_hi_and_version)
    result.Data[6] = (result.Data[6] & 0x0F) | 0x40;
    
    // Set variant (bits 6-7 of clock_seq_hi_and_reserved to 10)
    result.Data[8] = (result.Data[8] & 0x3F) | 0x80;
    
    return result;
}

LSTD_END_NAMESPACE
