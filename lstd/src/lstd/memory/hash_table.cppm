module;

#include "../common.h"

export module lstd.hash_table;

export import lstd.memory;
export import lstd.hash;

LSTD_BEGIN_NAMESPACE

// I hate C++
template <typename HashTableT>
using key_t = typename types::remove_pointer_t<decltype(HashTableT::Keys)>;

template <typename HashTableT>
using value_t = typename types::remove_pointer_t<decltype(HashTableT::Values)>;

//
// This hash table stores all entries in a contiguous array, for good performance when looking up things. Some tables
// work by storing linked lists of entries, but that can lead to many more cache misses.
//
// We store 3 arrays, one for the values, one for the keys and one for the hashed keys.
// Read the comment above reserve() for more information on how the arrays get allocated.
//
// When storing a value, we map its hash to a slot index and if that slot is free, we put the key and value there,
// otherwise we keep incrementing the slot index until we find an empty slot. Because the hash table can never be full, we
// are guaranteed to find a slot eventually.
//
// When looking up a value we perform the same process to find the correct slot.
//
// We use hash values to indicate whether slots are empty of removed. A hash of 0 means that slot is not used, so new
// values an be put there. A hash of 1 means that slot used to be valid, but has been removed. A hash of 2 or higher
// (FIRST_VALID_HASH) means this is a currently used slot.
//
// Whether we hash a key, if the result is less than 2, we just add 2 to it to put it in the valid range.
// This leads to possibly more collisions, but it's a small price to pay.
//
// The template parameter _BlockAlloc_ specifies whether the hashes, keys and values arrays are allocated
// all contiguously or by seperate allocation calls. You want to allocate them next to each other because that's good for the cache,
// but if the hash table is too large then the block won't fit in the cache anyways so you should consider setting this to false to reduce
// the size of the allocation request.
//
export {
    template <typename K, typename V, bool BlockAlloc = true>
    struct hash_table {
        static constexpr bool BLOCK_ALLOC = BlockAlloc;

        static constexpr s64 MINIMUM_SIZE     = 32;
        static constexpr s64 FIRST_VALID_HASH = 2;

        u64 *Hashes = null;
        K *Keys     = null;
        V *Values   = null;

        // Number of valid items
        s64 Count = 0;

        // Number of slots allocated
        // Do we get rid of this or not?
        s64 Allocated = 0;

        // Number of slots that can't be used (valid + removed items)
        s64 SlotsFilled = 0;

        // Returns a pointer to the value associated with _key_.
        // If the key doesn't exist, this adds a new element and returns it.
        V *operator[](const K &key) {
            auto [kp, vp] = find(this, key);
            if (vp) return vp;
            return add(this, key).Value;
        }
    };

    // types::is_same_template wouldn't work because hash_table contains a bool (and not a type) as a third template parameter.
    // At this point I hate C++
    template <typename>
    constexpr bool is_hash_table = false;

    template <typename K, typename V, bool B>
    constexpr bool is_hash_table<hash_table<K, V, B>> = true;

    template <typename T>
    concept any_hash_table = is_hash_table<types::remove_const_t<T>>;

    template <any_hash_table T>
    struct key_value_pair {
        key_t<T> *Key;
        value_t<T> *Value;
    };

    template <typename K, typename V, bool B>
    struct hash_table_iterator {
        using hash_table_t = hash_table<K, V, B>;

        hash_table_t *Table;
        s64 Index;

        hash_table_iterator(hash_table_t *table, s64 index = 0) : Table(table), Index(index) {
            assert(table);
            skip_empty_slots();  // Find the first pair
        }

        hash_table_iterator &operator++() {
            ++Index;
            skip_empty_slots();
            return *this;
        }

        hash_table_iterator operator++(s32) {
            hash_table_iterator pre = *this;
            ++*this;
            return pre;
        }

        bool operator==(hash_table_iterator other) const { return Table == other.Table && Index == other.Index; }
        bool operator!=(hash_table_iterator other) const { return !(*this == other); }

        key_value_pair<hash_table_t> operator*() {
            return {Table->Keys + Index, Table->Values + Index};
        }

        void skip_empty_slots() {
            for (; Index < Table->Allocated; ++Index) {
                if (Table->Hashes[Index] < Table->FIRST_VALID_HASH) continue;
                break;
            }
        }
    };

    auto begin(any_hash_table auto &table) { return hash_table_iterator(&table); }
    auto end(any_hash_table auto &table) { return hash_table_iterator(&table, table.Allocated); }

    // Makes sure the hash table has reserved enough space for at least n elements.
    // Note that it may reserve way more than required.
    // Reserves space equal to the next power of two bigger than _size_, starting at _MINIMUM_SIZE_.
    //
    // Allocates a buffer if the hash table doesn't already point to allocated memory (using the Context's allocator).
    // If _BLOCK_ALLOC_ is true it ensures that the allocated arrays are next to each other.
    //
    // You don't need to call this before using the hash table.
    // The first time an element is added to the hash table, it reserves with _MINIMUM_SIZE_ and no specified alignment.
    // You can call this before using the hash table to initialize the arrays with a custom alignment (if that's required).
    //
    // This is also called when adding an element and the hash table is more than half full (SlotsFilled * 2 >= Allocated).
    // In that case the _target_ is exactly _SlotsFilled_.
    // You may want to call this manually if you are adding a bunch of items and causing the hash table to reallocate a lot.
    template <typename K, typename V, bool B>
    void resize(hash_table<K, V, B> * table, s64 target, u32 alignment = 0) {
        if (table->SlotsFilled + target < table->Allocated) return;
        target = max<s64>(ceil_pow_of_2(target + table->SlotsFilled + 1), table->MINIMUM_SIZE);

        auto allocateNewBlock = [&]() {
            if constexpr (table->BLOCK_ALLOC) {
                s64 padding1 = 0, padding2 = 0;
                if (alignment != 0) {
                    padding1 = target * sizeof(u64) % alignment;
                    padding2 = target * sizeof(V) % alignment;
                }

                s64 sizeInBytes = target * (sizeof(u64) + sizeof(K) + sizeof(V)) + padding1 + padding2;

                byte *block   = malloc<byte>({.Count = sizeInBytes, .Alignment = alignment});
                table->Hashes = (u64 *) block;
                table->Keys   = (K *) (block + target * sizeof(u64) + padding1);
                table->Values = (V *) (block + target * (sizeof(u64) + sizeof(K)) + padding2);
                zero_memory(table->Hashes, target * sizeof(u64));
            } else {
                table->Hashes = malloc<u64>({.Count = target, .Alignment = alignment});
                table->Keys   = malloc<K>({.Count = target, .Alignment = alignment});
                table->Values = malloc<V>({.Count = target, .Alignment = alignment});
                zero_memory(table->Hashes, target * sizeof(u64));
            }
        };

        if (table->Allocated) {
            auto oldAlignment = ((allocation_header *) table->Hashes - 1)->Alignment;
            if (alignment == 0) {
                alignment = oldAlignment;
            } else {
                assert(alignment == oldAlignment && "Reserving with an alignment but the object already has arrays with a different alignment. Specify alignment 0 to automatically use the old one.");
            }

            auto *oldHashes   = table->Hashes;
            auto *oldKeys     = table->Keys;
            auto *oldValues   = table->Values;
            auto oldAllocated = table->Allocated;

            allocateNewBlock();

            // Add the old items
            For(range(oldAllocated)) {
                if (oldHashes[it] >= table->FIRST_VALID_HASH) add_prehashed(table, oldHashes[it], oldKeys[it], oldValues[it]);
            }

            free(oldHashes);

            if constexpr (!table->BLOCK_ALLOC) {
                free(oldKeys);
                free(oldValues);
            }
        } else {
            // It's impossible to have a view into a hash table (currently).
            // So there were no previous elements.
            assert(!table->Count);
            allocateNewBlock();
        }
        table->Allocated = target;
    }

    // Free any memory allocated by this object and reset count
    void free_table(any_hash_table auto *table) {
        if (table->Allocated) {
            free(table->Hashes);
            if constexpr (!table->BLOCK_ALLOC) {
                free(table->Keys);
                free(table->Values);
            }
        }

        table->Hashes      = null;
        table->Keys        = null;
        table->Values      = null;
        table->Count       = 0;
        table->SlotsFilled = 0;
        table->Allocated   = 0;
    }

    // Don't free the hash table, just destroy contents and reset count
    void reset(any_hash_table auto *table) {
        if (table->Allocated) {
            auto *p   = table->Hashes;
            auto *end = table->Hashes + table->Allocated;
            zero_memory(table.Hashes, table.Allocated * sizeof(u64));
        }

        table->Count       = 0;
        table->SlotsFilled = 0;
    }

    // Looks for key in the hash table using the given hash.
    // In normal _find_ we calculate the hash of the key using the global get_hash() specialized functions.
    // This method is useful if you have cached the hash.
    template <any_hash_table T>
    key_value_pair<T> find_prehashed(T * table, u64 hash, const key_t<T> &key) {
        if (!table->Count) return {null, null};

        s64 index = hash & table->Allocated - 1;
        For(range(table->Allocated)) {
            if (table->Hashes[index] == hash) {
                if (table->Keys[index] == key) {
                    return {table->Keys + index, table->Values + index};
                }
            }

            ++index;
            if (index >= table->Allocated) {
                index = 0;
            }
        }
        return {null, null};
    }

    // We calculate the hash of the key using the global get_hash() specialized functions.
    template <any_hash_table T>
    auto find(T * table, const key_t<T> &key) {
        return find_prehashed(table, get_hash(key), key);
    }

    // Adds key and value to the hash table using the given hash.
    // In normal _add_ we calculate the hash of the key using the global get_hash() specialized functions.
    // This method is useful if you have cached the hash.
    // Returns pointers to the added key and value.
    template <any_hash_table T>
    key_value_pair<T> add_prehashed(T * table, u64 hash, const key_t<T> &key, const value_t<T> &value) {
        // The + 1 here handles the case when the hash table size is 1 and you add the first item.
        if ((table->SlotsFilled + 1) * 2 >= table->Allocated) reserve(table, table->SlotsFilled);  // Make sure the hash table is never more than 50% full

        assert(table->SlotsFilled < table->Allocated);

        if (hash < table->FIRST_VALID_HASH) hash += table->FIRST_VALID_HASH;

        s64 index = hash & table->Allocated - 1;
        while (table->Hashes[index]) {
            ++index;
            if (index >= table->Allocated) index = 0;
        }

        ++table->Count;
        ++table->SlotsFilled;

        table->Hashes[index] = hash;
        new (table->Keys + index) key_t<T>(key);
        new (table->Values + index) value_t<T>(value);
        return {table->Keys + index, table->Values + index};
    }

    // Inserts an empty value at a specified key and returns pointers to the key and value in the buffers.
    //
    // This is useful for the following way to clone add an object (because by default we just shallow-copy the object).
    //
    // value_t toBeCloned = ...;
    // auto [kp, vp] = add(table, key);
    // *vp = clone(...);
    //
    // Because _add_ returns a pointer where the object is placed, clone() can place the deep copy there directly.
    template <any_hash_table T>
    key_value_pair<T> add(T * table, const key_t<T> &key) { return add(table, key, value_t<T>()); }

    // Inserts an empty key/value pair with a given hash.
    // Use the returned pointers to fill out the slots.
    // This is useful if you want to clone() the key and value and not just shallow copy them.
    template <any_hash_table T>
    key_value_pair<T> add(T * table, u64 hash) {
        return add_prehashed(table, hash, key_t<T>(), value_t<T>());
    }

    // Returns pointers to the added key and value.
    template <any_hash_table T>
    key_value_pair<T> add(T * table, const key_t<T> &key, const value_t<T> &value) {
        return add_prehashed(table, get_hash(key), key, value);
    }

    // This method is useful if you have cached the hash.
    template <any_hash_table T>
    key_value_pair<T> set_prehashed(T * table, u64 hash, const key_t<T> &key, const value_t<T> &value) {
        auto [kp, vp] = find_prehashed(table, hash, key);
        if (vp) {
            *vp = value;
            return {kp, vp};
        }
        return add(table, key, value);
    }

    template <any_hash_table T>
    key_value_pair<T> set(T * table, const key_t<T> &key, const value_t<T> &value) {
        return set_prehashed(table, get_hash(key), key, value);
    }

    // Returns true if the key was found and removed.
    // This method is useful if you have cached the hash.
    template <any_hash_table T>
    bool remove_prehashed(T * table, u64 hash, const key_t<T> &key) {
        auto [kp, vp] = find_prehashed(table, hash, key);
        if (vp) {
            s64 index            = vp - table->Values;
            table->Hashes[index] = 1;
            return true;
        }
        return false;
    }

    // Returns true if the key was found and removed.
    template <any_hash_table T>
    bool remove(T * table, const key_t<T> &key) {
        return remove_prehashed(table, get_hash(key), key);
    }

    // Returns true if the hash table has the given key.
    template <any_hash_table T>
    bool has(T * table, const key_t<T> &key) { return find(table, key).Key != null; }

    // Returns true if the hash table has the given key.
    // This method is useful if you have cached the hash.
    template <any_hash_table T>
    bool has_prehashed(T * table, u64 hash, const key_t<T> &key) { return find_prehashed(table, hash, key) != null; }

    template <any_hash_table T>
    bool operator==(const T &t, const T &u) {
        if (t.Count != u.Count) return false;

        for (auto [k, v] : t) {
            if (!has(u, *k)) return false;
            if (*v != *find(u, *k).Value) return false;
        }
        return true;
    }

    template <any_hash_table T>
    bool operator!=(const T &t, const T &u) { return !(t == u); }

    template <any_hash_table T>
    T clone(T * src) {
        T table;
        for (auto [k, v] : *src) add(&table, *k, *v);
        return table;
    }
}

LSTD_END_NAMESPACE
