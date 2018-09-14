#include "memory.h"

GU_BEGIN_NAMESPACE

#if defined GU_NO_CRT
void *CopyMemory(void *dest, void const *src, size_t num) {
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

void *MoveMemory(void *dest, void const *src, size_t num) {
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

void *FillMemory(void *dest, int value, size_t num) {
    byte *ptr = (byte *) dest;
    while (num-- > 0) {
        *ptr++ = value;
    }
    return dest;
}

s32 CompareMemory(const void *ptr1, const void *ptr2, size_t num) {
    for (; num--; ptr1++, ptr2++) {
        byte u1 = *(byte *) s1;
        byte u2 = *(byte *) s2;
        if (u1 != u2) {
            return (u1 - u2);
        }
    }
	// Memory is the same
    return 0;
}
#endif

GU_END_NAMESPACE
