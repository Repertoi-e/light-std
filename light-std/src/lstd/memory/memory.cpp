#include "memory.hpp"

LSTD_BEGIN_NAMESPACE

#if defined LSTD_NO_CRT
void *copy_memory(void *dest, void const *src, size_t num) {
    byte *d = (byte *) dest;
    byte const *s = (byte const *) src;

    if ((uptr_t) dest % sizeof(u32) == 0 && (uptr_t) src % sizeof(u32) == 0 && num % sizeof(u32) == 0) {
        for (size_t i = 0; i < num / sizeof(u32); i++) {
            ((u32 *) d)[i] = ((u32 *) s)[i];
        }
    } else {
        for (size_t i = 0; i < num; i++) {
            d[i] = s[i];
        }
    }
    return dest;
}

void *move_memory(void *dest, void const *src, size_t num) {
    byte *d = (byte *) dest;
    byte const *s = (byte const *) src;

    if (d <= s || d >= (s + num)) {
        // Non-Overlapping Buffers
        while (num--) {
            *d++ = *s++;
        }
    } else {
        // Overlapping Buffers
        d += num - 1;
        s += num - 1;

        while (num--) {
            *d-- = *s--;
        }
    }
    return dest;
}

void *fill_memory(void *dest, int value, size_t num) {
    byte *ptr = (byte *) dest;
    while (num-- > 0) {
        *ptr++ = value;
    }
    return dest;
}

s32 compare_memory(const void *ptr1, const void *ptr2, size_t num) {
    const byte *s1 = (const byte *) ptr1;
    const byte *s2 = (const byte *) ptr2;

    while (num-- > 0) {
        if (*s1++ != *s2++) return s1[-1] < s2[-1] ? -1 : 1;
    }

    // The memory regions match
    return 0;
}
#endif

LSTD_END_NAMESPACE
