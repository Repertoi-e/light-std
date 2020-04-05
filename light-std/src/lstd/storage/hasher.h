#pragma once

#include "../common.h"

LSTD_BEGIN_NAMESPACE

//
// Hasher based on Yann Collet's descriptions, see http://cyan4973.github.io/xxHash/
//
// Example use:
//    hasher h(..seed..);
//    h.add(&value);
//    ...
//    uptr_t result = h.get_hash();

#if BITS == 64
struct hasher {
    // Temporarily store up to 31 bytes between multiple add() calls
    static constexpr size_t MAX_BUFFER_SIZE = 31 + 1;

    char Buffer[MAX_BUFFER_SIZE]{};
    char *BufferPtr = Buffer;
    char *BufferEnd = Buffer + MAX_BUFFER_SIZE;

    u64 ByteLength = 0;

    u64 State[4]{};

    constexpr hasher(u64 seed) {
        State[0] = seed + 11400714785074694791ULL + 14029467366897019727ULL;
        State[1] = seed + 14029467366897019727ULL;
        State[2] = seed;
        State[3] = seed - 11400714785074694791ULL;
    }

    constexpr bool add(const char *data, size_t size) {
        if (!data) return false;

        ByteLength += size;

        if (BufferPtr + size < BufferEnd) {
            copy_memory_constexpr(BufferPtr, data, size);
            BufferPtr += size;
            return true;
        }

        if (BufferPtr != Buffer) {
            size_t available = BufferEnd - BufferPtr;
            copy_memory_constexpr(BufferPtr, data, available);
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

    constexpr u64 hash() {
        u64 result = 0;
        if (ByteLength >= MAX_BUFFER_SIZE) {
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

        result += ByteLength;

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
    constexpr void process(const void *data) {
        auto *block = (const u64 *) data;
        State[0] = rotate_left_64(State[0] + block[0] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
        State[1] = rotate_left_64(State[1] + block[1] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
        State[2] = rotate_left_64(State[2] + block[2] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
        State[3] = rotate_left_64(State[3] + block[3] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
    }
};
#else
struct hasher {
    // Temporarily store up to 31 bytes between multiple add() calls
    static constexpr size_t MAX_BUFFER_SIZE = 15 + 1;

    char Buffer[MAX_BUFFER_SIZE];
    char *BufferPtr = Buffer;
    char *BufferEnd = Buffer + MAX_BUFFER_SIZE;

    u32 ByteLength = 0;

    u32 State[4];

    constexpr hasher(u32 seed) {
        State[0] = seed + 2654435761U + 2246822519U;
        State[1] = seed + 2246822519U;
        State[2] = seed;
        State[3] = seed - 2654435761U;
    }

    template <typename T>
    constexpr bool add(const char *data, size_t size) {
        if (!data) return false;

        ByteLength += size;

        if (BufferPtr + size < BufferEnd) {
            copy_memory_constexpr(BufferPtr, data, size);
            BufferPtr += size;
            return true;
        }

        if (BufferPtr != Buffer) {
            size_t available = BufferEnd - BufferPtr;
            copy_memory_constexpr(BufferPtr, data, available);
            data += available;

            process(Buffer);
        }

        const char *stop = data + size - MAX_BUFFER_SIZE;
        while (data <= stop) {
            process(data);
            data += 16;
        }

        copy_memory(Buffer, data, size);
        BufferPtr = Buffer + size;
        return true;
    }

    constexpr u64 hash() {
        u32 result = ByteLength;

        if (totalLength >= MaxBufferSize) {
            result += rotate_left_32(State[0], 1);
            result += rotate_left_32(State[1], 7);
            result += rotate_left_32(State[2], 12);
            result += rotate_left_32(State[3], 18);
        } else {
            result += State[2] + 374761393U;
        }

        auto *p = Buffer;
        if (p + 4 <= BufferPtr) {
            result = rotate_left_32(result + (*(u32 *) p) * 3266489917U, 17);
            result *= 668265263U;
            p += 4;
        }

        while (p != BufferPtr) {
            result = rotate_left_32(result + (*p++) * 374761393U, 11) * 2654435761U;
        }

        result ^= result >> 15;
        result *= 2246822519U;
        result ^= result >> 13;
        result *= 3266489917U;
        result ^= result >> 16;
        return result;
    }

   private:
    constexpr void process(const void *data) {
        auto *block = (const u32 *) data;
        State[0] = rotate_left_32(State[0] + block[0] * 2246822519U, 13) * 2654435761U;
        State[1] = rotate_left_32(State[1] + block[1] * 2246822519U, 13) * 2654435761U;
        State[2] = rotate_left_32(State[2] + block[2] * 2246822519U, 13) * 2654435761U;
        State[3] = rotate_left_32(State[3] + block[3] * 2246822519U, 13) * 2654435761U;
    }
};
#endif

LSTD_END_NAMESPACE
