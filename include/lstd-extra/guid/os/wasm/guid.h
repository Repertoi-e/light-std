#pragma once

#include "../guid_common.h"
#include "lstd/stack_array.h"
#include "lstd/os.h"

LSTD_BEGIN_NAMESPACE

// Simple xorshift128+ PRNG state (per-thread to avoid synchronization)
struct prng_state {
    u64 s[2];
    bool initialized;
};

static thread_local prng_state g_prng = {{0, 0}, false};

static void init_prng() {
    if (g_prng.initialized) return;
    
    // Seed with high-resolution time and thread ID
    time_t now = os_get_time();
    u64 tid = os_get_current_thread_id();
    
    // Mix the bits to create two seed values
    g_prng.s[0] = (u64)now ^ (tid << 32);
    g_prng.s[1] = ((u64)now << 32) ^ tid ^ 0x9e3779b97f4a7c15ULL;
    
    // If both seeds are zero (unlikely), use a non-zero default
    if (g_prng.s[0] == 0 && g_prng.s[1] == 0) {
        g_prng.s[0] = 0x853c49e6748fea9bULL;
        g_prng.s[1] = 0xda3e39cb94b95bdbULL;
    }
    
    g_prng.initialized = true;
}

// xorshift128+ algorithm - fast and good quality PRNG
static u64 next_random() {
    init_prng();
    
    u64 s1 = g_prng.s[0];
    u64 s0 = g_prng.s[1];
    g_prng.s[0] = s0;
    s1 ^= s1 << 23;
    g_prng.s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
    return g_prng.s[1] + s0;
}

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
