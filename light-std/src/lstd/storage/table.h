#pragma once

#include "../memory/allocator.h"
#include "hash.h"
#include "owner_pointers.h"

LSTD_BEGIN_NAMESPACE

template <typename Key, typename Value, bool Const>
struct table_iterator;

// This hash table stores all entries in a contiguous array, for good perofrmance when looking up things. Some tables
// work by storing linked lists of entries, but that can lead to many more cache misses.
//
// We store 3 arrays, one for the values, one for the keys and one for the hashed keys.
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
template <typename K, typename V>
struct table {
    using key_t = K;
    using value_t = V;

    static constexpr size_t MINIMUM_SIZE = 32;
    static constexpr size_t FIRST_VALID_HASH = 2;

    // Number of valid items
    size_t Count = 0;

    // Number of slots allocated
    size_t Reserved = 0;

    // Number of slots that can't be used (valid or removed items)
    size_t SlotsFilled = 0;

    uptr_t *Hashes = null;
    key_t *Keys = null;
    value_t *Values = null;

    table() = default;
    ~table() { release(); }

    // Makes sure the table has reserved enough space for at least n elements.
    // Note that it may reserve way more than required.
    // Reserves space equal to the next power of two bigger than _size_, starting at _MINIMUM_SIZE_.
    //
    // Allocates a buffer if the table doesn't already point to reserved memory
    // (using the Context's allocator).
    void reserve(size_t target) {
        if (SlotsFilled + target < Reserved) return;
        target = MAX<size_t>(CEIL_POW_OF_2(target + SlotsFilled + 1), 8);

        auto *oldHashes = Hashes;
        auto *oldKeys = Keys;
        auto *oldValues = Values;
        auto oldReserved = Reserved;

        // The owner will change with the next line, but we need it later to decide if we need to delete the old arrays
        bool wasOwner = is_owner();

        Hashes = encode_owner(new (Context.Alloc, DO_INIT_FLAG) uptr_t[target + 1], this);
        Keys = new key_t[target];
        Values = new value_t[target];
        Reserved = target;

        For(range(oldReserved)) {
            if (oldHashes[it] < FIRST_VALID_HASH) continue;
            move_add(oldKeys + it, oldValues + it);
        }

        if (wasOwner) {
            delete[]((char *) oldHashes - POINTER_SIZE);
            delete[] oldKeys;
            delete[] oldValues;
        }
    }

    // Free any memory allocated by this object and reset count
    void release() {
        if (is_owner()) {
            delete[]((char *) Hashes - POINTER_SIZE);
            delete[] Keys;
            delete[] Values;
        }
        Hashes = null;
        Keys = null;
        Values = null;
        Count = SlotsFilled = Reserved = 0;
    }

    // Don't free the table, just destroy contents and reset count
    void reset() {
        // PODs may have destructors, although the C++ standard's definition forbids them to have non-trivial ones.
        if (is_owner()) {
            auto *p = Hashes, *end = Hashes + Reserved;
            size_t index = 0;
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

    value_t *find(const key_t &key) {
        if (!Reserved) return null;

        uptr_t hash = get_hash(key);
        size_t index = hash & (Reserved - 1);
        For(range(Reserved)) {
            if (Hashes[index] == hash) {
                if (Keys[index] == key) {
                    return Values + index;
                }
            }

            ++index;
            if (index >= Reserved) {
                index = 0;
            }
        }
        return null;
    }

    value_t *add(const key_t &key, const value_t &value) {
        // The + 1 here handles the case when the table size is 1 and you add the first item.
        if ((SlotsFilled + 1) * 2 >= Reserved) reserve(SlotsFilled * 2);

        assert(SlotsFilled < Reserved);

        uptr_t hash = get_hash(key);
        if (hash < FIRST_VALID_HASH) hash += FIRST_VALID_HASH;

        size_t index = hash & (Reserved - 1);
        while (Hashes[index]) {
            ++index;
            if (index >= Reserved) index = 0;
        }

        ++Count;
        ++SlotsFilled;

        Hashes[index] = hash;
        new (Keys + index) key_t(key);
        new (Values + index) value_t(value);
        return Values + index;
    }

    // Same as _add_ but calls _move()_ on _key_ and _value_ when adding them to the array.
    value_t *move_add(key_t *key, value_t *value) {
        // The + 1 here handles the case when the table size is 1 and you add the first item.
        if ((SlotsFilled + 1) * 2 >= Reserved) reserve(SlotsFilled * 2);

        assert(SlotsFilled < Reserved);

        uptr_t hash = get_hash(*key);
        if (hash < FIRST_VALID_HASH) hash += FIRST_VALID_HASH;

        size_t index = hash & (Reserved - 1);
        while (Hashes[index]) {
            ++index;
            if (index >= Reserved) index = 0;
        }

        ++Count;
        ++SlotsFilled;

        Hashes[index] = hash;
        move(Keys + index, key);
        move(Values + index, value);
        return Values + index;
    }

    value_t *set(const key_t &key, const value_t &value) {
        auto *ptr = find(key);
        if (ptr) {
            *ptr = value;
            return ptr;
        }
        return add(key, value);
    }

    bool remove(const key_t &key) {
        auto *ptr = find(key);
        if (ptr) {
            size_t index = ptr - Values;
            Hashes[index] = 1;
            return true;
        }
        return false;
    }

    bool has(const key_t &key) const { return find(key) != null; }

    // Returns true if this object has any memory allocated by itself
    bool is_owner() const { return Reserved && decode_owner<table>(Hashes) == this; }

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
    // If the key doesn't exist, put a blank value and return a pointer to that.
    value_t *operator[](const key_t &key) {
        auto *ptr = find(key);
        if (ptr) return ptr;
        return add(key, value_t());
    }
};

template <typename Key, typename Value, bool Const>
struct table_iterator {
    using table_t = type_select_t<Const, const table<Key, Value>, table<Key, Value>>;

    table_t *Parent;
    size_t Index;

    table_iterator(table_t *parent, size_t index = 0) : Parent(parent), Index(index) {
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

    bool operator==(table_iterator other) const { return Parent == other.Parent && Index == other.Index; }
    bool operator!=(table_iterator other) const { return !(*this == other); }

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

template <typename K, typename V>
typename table<K, V>::iterator table<K, V>::begin() {
    return table<K, V>::iterator(this);
}

template <typename K, typename V>
typename table<K, V>::iterator table<K, V>::end() {
    return table<K, V>::iterator(this, Reserved);
}

template <typename K, typename V>
typename table<K, V>::const_iterator table<K, V>::begin() const {
    return table<K, V>::const_iterator(this);
}

template <typename K, typename V>
typename table<K, V>::const_iterator table<K, V>::end() const {
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

template <typename K, typename V>
table<K, V> *move(table<K, V> *dest, table<K, V> *src) {
    dest->release();
    *dest = *src;

    if (!src->is_owner()) return;

    // Transfer ownership
    change_owner(src->Hashes, dest);
    change_owner(dest->Hashes, dest);
    return dest;
}

LSTD_END_NAMESPACE
