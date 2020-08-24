#pragma once

#include "../internal/common.h"

LSTD_BEGIN_NAMESPACE

//
// Hasher based on Yann Collet's descriptions, see http://cyan4973.github.io/xxHash/
//
// Example use:
//    hasher h(..seed..);
//    h.add(&value);
//    ...
//    u64 result = h.get_hash();
// @Speed: Not so fast atm..

struct hasher {
    // Temporarily store up to 31 bytes between multiple add() calls
    static constexpr s64 MAX_BUFFER_SIZE = 31 + 1;

    char Buffer[MAX_BUFFER_SIZE]{};
    char *BufferPtr = Buffer;
    char *BufferEnd = Buffer + MAX_BUFFER_SIZE;

    u64 Count = 0;

    u64 State[4]{};

    hasher(u64 seed) {
        State[0] = seed + 11400714785074694791ULL + 14029467366897019727ULL;
        State[1] = seed + 14029467366897019727ULL;
        State[2] = seed;
        State[3] = seed - 11400714785074694791ULL;
    }

    // @Speed: SIMD this shit
    bool add(const char *data, s64 size) {
        if (!data) return false;

        Count += size;

        if (BufferPtr + size < BufferEnd) {
            copy_memory(BufferPtr, data, size);
            BufferPtr += size;
            return true;
        }

        if (BufferPtr != Buffer) {
            s64 available = BufferEnd - BufferPtr;
            copy_memory(BufferPtr, data, available);
            data += available;

            process(Buffer);
        }

        const char *stop = data + size - MAX_BUFFER_SIZE;
        while (data <= stop) {
            process(data);
            data += 32;
        }

        copy_memory(Buffer, data, size);
        BufferPtr = Buffer + size;
        return true;
    }

    u64 hash() {
        u64 result = 0;
        if (Count >= MAX_BUFFER_SIZE) {
            result += rotate_left_64(State[0], 1);
            result += rotate_left_64(State[1], 7);
            result += rotate_left_64(State[2], 12);
            result += rotate_left_64(State[3], 18);

            result ^= rotate_left_64(State[0] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
            result *= 11400714785074694791ULL;
            result += 9650029242287828579ULL;

            result ^= rotate_left_64(State[1] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
            result *= 11400714785074694791ULL;
            result += 9650029242287828579ULL;

            result ^= rotate_left_64(State[2] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
            result *= 11400714785074694791ULL;
            result += 9650029242287828579ULL;

            result ^= rotate_left_64(State[3] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
            result *= 11400714785074694791ULL;
            result += 9650029242287828579ULL;
        } else {
            result = State[2] + 2870177450012600261ULL;
        }

        result += Count;

        auto *p = Buffer;
        while (p + 8 < BufferPtr) {
            result = rotate_left_64(result ^ (rotate_left_64(*(u64 *) p * 14029467366897019727ULL, 31)), 27);
            result *= 11400714785074694791ULL;
            result += 9650029242287828579ULL;

            p += 8;
        }

        if (p + 4 <= BufferPtr) {
            result = rotate_left_64(result ^ (*(u32 *) p) * 11400714785074694791ULL, 23);
            result *= 14029467366897019727ULL;
            result += 1609587929392839161ULL;
            p += 4;
        }

        while (p != BufferPtr) {
            result = rotate_left_64(result ^ (*p++) * 2870177450012600261ULL, 11) * 11400714785074694791ULL;
        }

        result ^= result >> 33;
        result *= 14029467366897019727ULL;
        result ^= result >> 29;
        result *= 1609587929392839161ULL;
        result ^= result >> 32;
        return result;
    }

   private:
    void process(const void *data) {
        auto *block = (const u64 *) data;
        State[0] = rotate_left_64(State[0] + block[0] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
        State[1] = rotate_left_64(State[1] + block[1] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
        State[2] = rotate_left_64(State[2] + block[2] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
        State[3] = rotate_left_64(State[3] + block[3] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
    }
};

LSTD_END_NAMESPACE
