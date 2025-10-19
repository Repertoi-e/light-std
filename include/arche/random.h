#pragma once

#include "os.h"

//
// Simple xorshift128+ PRNG random generator (thread-local to avoid synchronization).
//
// Initialized with high-resolution time and thread ID.
// Not cryptographically secure, but fast and good enough for most purposes.
//

struct prng_state {
    u64  s[2];
    bool initialized;
};

inline thread_local prng_state g_PRNG = {{0, 0}, false};

inline void init_prng() {
    if (g_PRNG.initialized) return;

    // Seed with high-resolution time and thread ID
    time_t now = os_get_time();
    u64    tid = os_get_current_thread_id();

    // Mix the bits to create two seed values
    g_PRNG.s[0] = (u64)now ^ (tid << 32);
    g_PRNG.s[1] = ((u64)now << 32) ^ tid ^ 0x9e3779b97f4a7c15ULL;

    // If both seeds are zero (unlikely), use a non-zero default
    if (g_PRNG.s[0] == 0 && g_PRNG.s[1] == 0) {
        g_PRNG.s[0] = 0x853c49e6748fea9bULL;
        g_PRNG.s[1] = 0xda3e39cb94b95bdbULL;
    }

    g_PRNG.initialized = true;
}

// xorshift128+ algorithm - fast and good quality PRNG
inline u64 next_random() {
    init_prng();

    u64 s1      = g_PRNG.s[0];
    u64 s0      = g_PRNG.s[1];
    g_PRNG.s[0] = s0;
    s1 ^= s1 << 23;
    g_PRNG.s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
    return g_PRNG.s[1] + s0;
}