#pragma once

#include "../memory/allocator.h"
#include "hash.h"

LSTD_BEGIN_NAMESPACE

template <typename Key, typename Value, bool Const>
struct table_iterator;

// This hash table stores all entries in a contiguous array, for good performance when looking up things. Some tables
// work by storing linked lists of entries, but that can lead to many more cache misses.
//
// We store 3 arrays, one for the values, one for the keys and one for the hashed keys.
// Read the comment above reserve() for more information on how the arrays get allocated.
//
// When storing a value, we map its hash to a slot index and if that slot is free, we put the key and value there,
// otherwise we keep incrementing the slot index until we find an empty slot. Because the table can never be full, we
// are guaranteed to find a slot eventually.
//
// When looking up a value we perform the same process to find the correct slot.
//
// We use hash values to indicate whether slots are empty of removed. A hash of 0 means that slot is not used, so new
// values an be put there. A hash of 1 means that slot used to be valid, but has been removed. A hash of 2 or h igher
// (FIRST_VALID_HASH) means this is a currently used slot.
//
// Whether we hash a key, if the result is less than 2, we just add 2 to it to put it in the valid range.
// This leads to possibly more collisions, but it's a small price to pay.
//
// The template parameter _BlockAlloc_ specifies whether the hashes, keys and values arrays are allocated
// all contiguously or by seperate allocation calls. You want to allocate them next to each other because that's good for the cache,
// but if the table is too large then the block won't fit in the cache anyways so you should consider setting this to false to reduce
// the size of the allocation request.
template <typename K, typename V, bool BlockAlloc = true>
struct table {
    using key_t = K;
    using value_t = V;

    static constexpr bool BLOCK_ALLOC = BlockAlloc;

    static constexpr s64 MINIMUM_SIZE = 32;
    static constexpr s64 FIRST_VALID_HASH = 2;

    // Number of valid items
    s64 Count = 0;

    // Number of slots allocated
    s64 Reserved = 0;

    // Number of slots that can't be used (valid + removed items)
    s64 SlotsFilled = 0;

    u64 *Hashes = null;
    key_t *Keys = null;
    value_t *Values = null;

    table() = default;

    // We don't use destructors for freeing memory anymore.
    // ~table() { release(); }

    // Makes sure the table has reserved enough space for at least n elements.
    // Note that it may reserve way more than required.
    // Reserves space equal to the next power of two bigger than _size_, starting at _MINIMUM_SIZE_.
    //
    // Allocates a buffer if the table doesn't already point to reserved memory (using the Context's allocator).
    // If _BLOCK_ALLOC_ is true it ensures that the allocated arrays are next to each other.
    //
    // You don't need to call this before using the table.
    // The first time an element is added to the table, it reserves with _MINIMUM_SIZE_ and no specified alignment.
    // You can call this before using the table to initialize the arrays with a custom alignment (if that's required).
    //
    // This is also called when adding an element and the table is half full (SlotsFilled * 2 == Reserved).
    // In that case the _target_ is exactly _SlotsFilled_ * 2.
    // You may want to call this manually if you are adding a bunch of items and causing the table to reallocate a lot.
    void reserve(s64 target, u32 alignment = 0) {
        if (SlotsFilled + target < Reserved) return;
        target = max<s64>(ceil_pow_of_2(target + SlotsFilled + 1), MINIMUM_SIZE);

        if (Reserved) {
            auto oldAlignment = ((allocation_header *) Hashes - 1)->Alignment;
            if (alignment == 0) {
                alignment = oldAlignment;
            } else {
                assert(alignment == oldAlignment && "Reserving with an alignment but the object already has arrays with a different alignment. Specify alignment 0 to automatically use the old one.");
            }

            if constexpr (BLOCK_ALLOC) {
                auto *oldHashes = Hashes;
                auto *oldKeys = Keys;
                auto *oldValues = Values;
                auto oldReserved = Reserved;

                s64 padding1 = 0, padding2 = 0;
                if (alignment != 0) {
                    padding1 = (target * sizeof(u64)) % alignment;
                    padding2 = (target * sizeof(value_t)) % alignment;
                }

                s64 sizeInBytes = target * (sizeof(u64) + sizeof(key_t) + sizeof(value_t)) + padding1 + padding2;

                char *block = reallocate_array((char *) Hashes, sizeInBytes);
                Hashes = (u64 *) block;
                Keys = (key_t *) (block + target * sizeof(u64) + padding1);
                Values = (value_t *) (block + target * (sizeof(u64) + sizeof(key_t)) + padding2);
                zero_memory(Hashes, target * sizeof(u64));

                // Copy the old values in reverse because the block might not have moved in memory.
                // copy_memory handles moving when the src and dest buffers overlap.
                copy_memory(Values, oldValues, oldReserved * sizeof(value_t));
                copy_memory(Keys, oldKeys, oldReserved * sizeof(key_t));
                copy_memory(Hashes, oldHashes, oldReserved * sizeof(u64));
            } else {
                Hashes = reallocate_array(Hashes, target, DO_INIT_0);
                Keys = reallocate_array(Keys, target);
                Values = reallocate_array(Values, target);
            }
        } else {
            // It's impossible to have a view into a table (currently).
            // So there were no previous elements.
            assert(!Count);

            if constexpr (BLOCK_ALLOC) {
                s64 padding1 = 0, padding2 = 0;
                if (alignment != 0) {
                    padding1 = (target * sizeof(u64)) % alignment;
                    padding2 = (target * sizeof(value_t)) % alignment;
                }

                s64 sizeInBytes = target * (sizeof(u64) + sizeof(key_t) + sizeof(value_t)) + padding1 + padding2;

                char *block = allocate_array_aligned(char, sizeInBytes, alignment);
                Hashes = (u64 *) block;
                Keys = (key_t *) (block + target * sizeof(u64) + padding1);
                Values = (value_t *) (block + target * (sizeof(u64) + sizeof(key_t)) + padding2);
                zero_memory(Hashes, target * sizeof(u64));
            } else {
                Hashes = allocate_array_aligned(u64, target, alignment, DO_INIT_0);
                Keys = allocate_array_aligned(key_t, target, alignment);
                Values = allocate_array_aligned(value_t, target, alignment);
            }
        }
        Reserved = target;
    }

    // Free any memory allocated by this object and reset count
    void release() {
        if (Reserved) {
            if constexpr (BLOCK_ALLOC) {
                free(Hashes);
            } else {
                free(Hashes);
                free(Keys);
                free(Values);
            }
        }
        Hashes = null;
        Keys = null;
        Values = null;
        Count = SlotsFilled = Reserved = 0;
    }

    // Don't free the table, just destroy contents and reset count
    void reset() {
        // PODs may have destructors, although the C++ standard's definition forbids them to have non-trivial ones.
        if (Reserved) {
            // @TODO: Factor this into uninitialize_block() function and use it in free() as well
            auto *p = Hashes, *end = Hashes + Reserved;
            s64 index = 0;
            while (p != end) {
                if (*p) {
                    Keys[index].~key_t();
                    Values[index].~value_t();
                    *p = 0;
                }
                ++index, ++p;
            }
        }
        Count = SlotsFilled = 0;
    }

    // We calculate the hash of the key using the global get_hash() specialized functions.
    pair<key_t *, value_t *> find(const key_t &key) {
        return find_prehashed(get_hash(key), key);
    }

    // Looks for key in the table using the given hash.
    // In normal _find_ we calculate the hash of the key using the global get_hash() specialized functions.
    // This method is useful if you have cached the hash.
    pair<key_t *, value_t *> find_prehashed(u64 hash, const key_t &key) {
        if (!Reserved) return {null, null};

        s64 index = hash & (Reserved - 1);
        For(range(Reserved)) {
            if (Hashes[index] == hash) {
                if (Keys[index] == key) {
                    return {Keys + index, Values + index};
                }
            }

            ++index;
            if (index >= Reserved) {
                index = 0;
            }
        }
        return {null, null};
    }

    // Inserts an empty value at a specified key and returns pointers to the key and value in the buffers.
    //
    // This is useful for the following way to clone add an object (because by default we just shallow-copy the object).
    //
    // value_t toBeCloned = ...;
    // auto [kp, vp] = table.add(key);
    // clone(vp, toBeCloned);
    //
    // Because _add_ returns a pointer where the object is placed, clone() can place the deep copy there directly.
    //
    // We calculate the hash of the key using the global get_hash() specialized functions.
    pair<key_t *, value_t *> add(const key_t &key) { return add(key, value_t()); }

    // Inserts an empty key/value pair with a given hash.
    // Use the returned pointers to fill out the slots.
    // This is useful if you want to clone() the key and value and not just shallow copy them.
    pair<key_t *, value_t *> add(u64 hash) {
        return add_prehashed(has, key_t(), value_t());
    }

    // We calculate the hash of the key using the global get_hash() specialized functions.
    // Returns pointers to the added key and value.
    pair<key_t *, value_t *> add(const key_t &key, const value_t &value) {
        return add_prehashed(get_hash(key), key, value);
    }

    // Adds key and value to the table using the given hash.
    // In normal _add_ we calculate the hash of the key using the global get_hash() specialized functions.
    // This method is useful if you have cached the hash.
    // Returns pointers to the added key and value.
    pair<key_t *, value_t *> add_prehashed(u64 hash, const key_t &key, const value_t &value) {
        // The + 1 here handles the case when the table size is 1 and you add the first item.
        if ((SlotsFilled + 1) * 2 >= Reserved) reserve(SlotsFilled * 2);

        assert(SlotsFilled < Reserved);

        if (hash < FIRST_VALID_HASH) hash += FIRST_VALID_HASH;

        s64 index = hash & (Reserved - 1);
        while (Hashes[index]) {
            ++index;
            if (index >= Reserved) index = 0;
        }

        ++Count;
        ++SlotsFilled;

        Hashes[index] = hash;
        new (Keys + index) key_t(key);
        new (Values + index) value_t(value);
        return {Keys + index, Values + index};
    }

    // We calculate the hash of the key using the global get_hash() specialized functions.
    pair<key_t *, value_t *> set(const key_t &key, const value_t &value) {
        return set_prehashed(get_hash(key), key, value);
    }

    // In normal _set_ we calculate the hash of the key using the global get_hash() specialized functions.
    // This method is useful if you have cached the hash.
    pair<key_t *, value_t *> set_prehashed(u64 hash, const key_t &key, const value_t &value) {
        auto [kp, vp] = find_prehashed(hash, key);
        if (vp) {
            *vp = value;
            return {kp, vp};
        }
        return add(key, value);
    }

    // Returns true if the key was found and removed.
    // We calculate the hash of the key using the global get_hash() specialized functions.
    bool remove(const key_t &key) {
        return remove_prehashed(get_hash(key), key);
    }

    // Returns true if the key was found and removed.
    // In normal _remove_ we calculate the hash of the key using the global get_hash() specialized functions.
    // This method is useful if you have cached the hash.
    bool remove_prehashed(u64 hash, const key_t &key) {
        auto *ptr = find(hash, key);
        if (ptr) {
            s64 index = ptr - Values;
            Hashes[index] = 1;
            return true;
        }
        return false;
    }

    // Returns true if the table has the given key.
    // We calculate the hash of the key using the global get_hash() specialized functions.
    bool has(const key_t &key) const { return find(key) != null; }

    // Returns true if the table has the given key.
    // In normal _hash_ we calculate the hash of the key using the global get_hash() specialized functions.
    // This method is useful if you have cached the hash.
    bool has_prehashed(u64 hash, const key_t &key) const { return find_prehashed(hash, key) != null; }

    //
    // Iterator:
    //
    using iterator = table_iterator<key_t, value_t, false>;
    using const_iterator = table_iterator<key_t, value_t, true>;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    //
    // Operators:
    //

    // Returns a pointer to the value associated with _key_.
    // If the key doesn't exist, this adds a new element and returns it.
    value_t *operator[](const key_t &key) {
        auto [kp, vp] = find(key);
        if (vp) return vp;
        return add(key, value_t()).Second;
    }
};

template <typename Key, typename Value, bool Const>
struct table_iterator {
    using table_t = type_select_t<Const, const table<Key, Value>, table<Key, Value>>;

    table_t *Parent;
    s64 Index;

    table_iterator(table_t *parent, s64 index = 0) : Parent(parent), Index(index) {
        assert(parent);

        // Find the first pair
        skip_empty_slots();
    }

    table_iterator &operator++() {
        ++Index;
        skip_empty_slots();
        return *this;
    }

    table_iterator operator++(int) {
        table_iterator pre = *this;
        ++(*this);
        return pre;
    }

    bool operator==(const table_iterator &other) const { return Parent == other.Parent && Index == other.Index; }
    bool operator!=(const table_iterator &other) const { return !(*this == other); }

    template <bool NotConst = !Const>
    enable_if_t<NotConst, pair<Key *, Value *>> operator*() {
        return {Parent->Keys + Index, Parent->Values + Index};
    }

    template <bool NotConst = !Const>
    enable_if_t<!NotConst, pair<const Key *, const Value *>> operator*() const {
        return {Parent->Keys + Index, Parent->Values + Index};
    }

   private:
    void skip_empty_slots() {
        for (; Index < Parent->Reserved; ++Index) {
            if (Parent->Hashes[Index] < table_t::FIRST_VALID_HASH) continue;
            break;
        }
    }
};

template <typename K, typename V, bool BlockAlloc>
typename table<K, V, BlockAlloc>::iterator table<K, V, BlockAlloc>::begin() {
    return table<K, V>::iterator(this);
}

template <typename K, typename V, bool BlockAlloc>
typename table<K, V, BlockAlloc>::iterator table<K, V, BlockAlloc>::end() {
    return table<K, V>::iterator(this, Reserved);
}

template <typename K, typename V, bool BlockAlloc>
typename table<K, V, BlockAlloc>::const_iterator table<K, V, BlockAlloc>::begin() const {
    return table<K, V>::const_iterator(this);
}

template <typename K, typename V, bool BlockAlloc>
typename table<K, V, BlockAlloc>::const_iterator table<K, V, BlockAlloc>::end() const {
    return table<K, V>::const_iterator(this, Reserved);
}

template <typename K, typename V>
table<K, V> *clone(table<K, V> *dest, const table<K, V> &src) {
    *dest = {};
    for (auto [key, value] : src) {
        dest->add(*key, *value);
    }
    return dest;
}

LSTD_END_NAMESPACE
