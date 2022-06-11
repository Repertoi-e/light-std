module;

#include "../common.h"

export module lstd.hash;

export import lstd.guid;
export import lstd.string;

//
// !!! THESE ARE NOT SUPPOSED TO BE CRYPTOGRAPHICALLY SECURE !!!
//
// This header provides a way to hash any type.
// If you want to implement a custom hash function for your type,
// implement it in global namespace like this:
//
// LSTD_BEGIN_NAMESPACE
// u64 get_hash(T value) { return ...; }
// LSTD_END_NAMESPACE
//

LSTD_BEGIN_NAMESPACE

export {
    //
    // Hasher based on Yann Collet's descriptions, see http://cyan4973.github.io/xxHash/
    //
    // Example use:
    //    hasher h(..seed..);
    //    h.add(&value);
    //    ...
    //    u64 result = h.get_hash();
    //
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

        // @Speed: SIMD
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
                result = rotate_left_64(result ^ rotate_left_64(*(u64 *) p * 14029467366897019727ULL, 31), 27);
                result *= 11400714785074694791ULL;
                result += 9650029242287828579ULL;

                p += 8;
            }

            if (p + 4 <= BufferPtr) {
                result = rotate_left_64(result ^ *(u32 *) p * 11400714785074694791ULL, 23);
                result *= 14029467366897019727ULL;
                result += 1609587929392839161ULL;
                p += 4;
            }

            while (p != BufferPtr) {
                result = rotate_left_64(result ^ *p++ * 2870177450012600261ULL, 11) * 11400714785074694791ULL;
            }

            result ^= result >> 33;
            result *= 14029467366897019727ULL;
            result ^= result >> 29;
            result *= 1609587929392839161ULL;
            result ^= result >> 32;
            return result;
        }

        void process(const void *data) {
            auto *block = (const u64 *) data;
            State[0]    = rotate_left_64(State[0] + block[0] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
            State[1]    = rotate_left_64(State[1] + block[1] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
            State[2]    = rotate_left_64(State[2] + block[2] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
            State[3]    = rotate_left_64(State[3] + block[3] * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
        }
    };

    // Hash for any type.
    // Floats are handled here.
    // The output depends on the endianness of the machine.
    template <typename T>
    constexpr u64 get_hash(const T &value) {
        hasher h(0);
        h.add((const char *) &value, sizeof(T));
        return h.hash();
    }

    // Partial specialization for pointers
    constexpr u64 get_hash(types::is_pointer auto value) { return (u64) value; }

    // @TODO: Hash for array_like

    // Partial specialization for arrays of known size
    template <types::is_array T>
    requires(types::is_array_of_known_bounds_v<T>) constexpr u64 get_hash(const T value) {
        hasher h(0);
        h.add((const char *) value, sizeof(types::remove_extent_t<T>) * types::extent_v<T>);
        return h.hash();
    }

// Hashes for integer types
#define TRIVIAL_HASH(T) \
    constexpr u64 get_hash(T value) { return (u64) value; }

    TRIVIAL_HASH(s8);
    TRIVIAL_HASH(u8);

    TRIVIAL_HASH(s16);
    TRIVIAL_HASH(u16);

    TRIVIAL_HASH(s32);
    TRIVIAL_HASH(u32);

    TRIVIAL_HASH(s64);
    TRIVIAL_HASH(u64);

    TRIVIAL_HASH(bool);

    // Hashing strings...
    u64 get_hash(string value) {
        u64 hash        = 5381;
        For(value) hash = ((hash << 5) + hash) + it;
        return hash;
    }

    // Hashing guids...
    u64 get_hash(guid value) {
        u64 hash             = 5381;
        For(value.Data) hash = (hash << 5) + hash + it;
        return hash;
    }
}

LSTD_END_NAMESPACE
