#pragma once

#include "../memory/allocator.h"
#include "hash.h"
#include "owner_pointers.h"

LSTD_BEGIN_NAMESPACE

template <typename Key, typename Value, bool Const>
struct table_iterator;

template <typename K, typename V>
struct table {
    // If your type can be copied byte by byte correctly,
    // then you can explicitly declare it as POD in global namespace with DECLARE_IS_POD(type, true)
    static_assert(is_pod_v<K> && is_pod_v<V> &&
                  "tables can only work with POD types, take a look at the type policy in common.h");

    using key_t = K;
    using value_t = V;

    static constexpr size_t MINIMUM_SIZE = 32;

    size_t Count = 0;
    size_t Reserved = 0;

    // The value that gets returned if a value is not found in _find()_
    // By default it's a default constructed value_t.
    // This value may be changed by the user for specific cases.
    value_t UnfoundValue = value_t();

    bool *_OccupancyMask = null;
    key_t *_Keys = null;
    value_t *_Values = null;
    uptr_t *_Hashes = null;

    table() = default;
    ~table() { release(); }

    // Makes sure the table has reserved enough space for at least n elements.
    // Note that it may reserve way more than required.
    // Reserves space equal to the next power of two bigger than _size_, starting at _MINIMUM_SIZE_.
    //
    // Allocates a buffer if the table doesn't already point to reserved memory
    // (using the Context's allocator by default).
    // You can also use this function to change the allocator of a table before using it.
    //    reserve(0, ...) is enough to allocate space for _MINIMUM_SIZE_ elements with the passed in allocator.
    //
    // For robustness, this function asserts if you pass an allocator, but the table has already
    // reserved a buffer with a *different* allocator.
    //
    // If the table points to reserved memory but doesn't own it, this function asserts.
    void reserve(size_t size, allocator alloc = {null, null}) {
        if (size < Reserved) return;

        if (!Reserved && size < (Count * 2)) {
            // Note: This may still not be enough space if the hash function produces a lot of collisions
            size += Count * 2;
        }

        size_t reserveTarget = MINIMUM_SIZE;
        while (reserveTarget < size) {
            reserveTarget *= 2;
        }

        if (Reserved) {
            assert(is_owner() && "Cannot resize a buffer that isn't owned by this table.");

            auto *actualOccupancyMask = (byte *) _OccupancyMask - POINTER_SIZE;

            if (alloc) {
                auto *header = (allocation_header *) actualOccupancyMask - 1;
                assert(alloc.Function == header->AllocatorFunction && alloc.Context == header->AllocatorContext &&
                       "Calling reserve() on a table that already has reserved a buffer but with a different "
                       "allocator. Call with null allocator to avoid that.");
            }
        }

        auto *oldOccupancyMask = _OccupancyMask;
        auto *oldKeys = _Keys;
        auto *oldValues = _Values;
        auto *oldHashes = _Hashes;

        size_t oldReserve = 0;
        if (oldOccupancyMask) oldReserve = ((allocation_header *) oldOccupancyMask - 1)->Size;

        _OccupancyMask = encode_owner((bool *) new (alloc) byte[(reserveTarget + 1) * POINTER_SIZE], this);
        _Keys = new (alloc) key_t[reserveTarget];
        _Values = new (alloc) value_t[reserveTarget];
        _Hashes = new (alloc) uptr_t[reserveTarget];

        For(range(oldReserve)) {
            if (oldOccupancyMask[it]) {
                put(oldKeys[it], oldValues[it]);
            }
        }

        if (oldReserve) {
            delete[] oldOccupancyMask;
            delete[] oldKeys;
            delete[] oldValues;
            delete[] oldHashes;
        }

        Reserved = reserveTarget;
    }

    // Free any memory allocated by this object and reset count
    void release() {
        reset();
        if (is_owner()) {
            delete[]((byte *) _OccupancyMask - POINTER_SIZE);
            delete[] _Keys;
            delete[] _Values;
            delete[] _Hashes;

            _OccupancyMask = null;
            _Keys = null;
            _Values = null;
            _Hashes = null;
            Reserved = 0;
        }
    }

    // Don't free the table, just destroy contents and reset count
    void reset() {
        bool *p = _OccupancyMask, *end = _OccupancyMask + Reserved;
        size_t index = 0;

        // PODs may have destructors, although the C++ standard's definition forbids them to have non-trivial ones.
        while (p != end) {
            if (*p) {
                _Keys[index].~key_t();
                _Values[index].~value_t();
                *p = false;
            }
            ++index, ++p;
        }

        Count = 0;
    }

    pair<value_t *, bool> find(const key_t &key) {
        uptr_t hashed = hash<key_t>::get(key);

        s32 index = find_index(key, hashed);
        if (index == -1) {
            return {null, false};
        }

        return {_Values + index, true};
    }

    bool put(const key_t &key, const value_t &value) {
        if (find(key).Second) return false;

        uptr_t hashed = hash<key_t>::get(key);

        s32 index = find_index(key, hashed);
        if (index == -1) {
            if (Count >= Reserved) {
                reserve(Reserved * 2);
            }
            assert(Count <= Reserved);

            index = (s32)(hashed % Reserved);

            // Resolve collision
            while (_OccupancyMask[index]) {
                ++index;
                if ((size_t) index >= Reserved) {
                    index = 0;
                }
            }

            Count++;
        }

        _OccupancyMask[index] = true;
        new (_Keys + index) key_t(key);
        new (_Values + index) value_t(value);
        _Hashes[index] = hashed;

        return true;
    }

    bool has(const key_t &key) const { return find(key).Second; }

    // Returns true if this object has any memory allocated by itself
    bool is_owner() const { return Reserved && decode_owner<table>(_OccupancyMask) == this; }

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
        auto found = find(key);
        if (found.Second) {
            return found.First;
        } else {
            put(key, UnfoundValue);
            found = find(key);
            assert(found.Second);
            return found.First;
        }
    }

   private:
    s32 find_index(const key_t &key, uptr_t hashed) {
        if (!Reserved) return -1;

        size_t index = hashed % Reserved;
        while (_OccupancyMask[index]) {
            if (_Hashes[index] == hashed) {
                if (_Keys[index] == key) {
                    return (s32) index;
                }
            }

            ++index;
            if (index >= Reserved) {
                index = 0;
            }
        }
        return -1;
    }
};

template <typename Key, typename Value, bool Const>
struct table_iterator {
    table<Key, Value> *Parent;
    s64 Index = 0;

    table_iterator(table<Key, Value> *table, s64 index = -1) : Parent(table), Index(index) {
        // Find the first pair
        ++(*this);
    }

    table_iterator &operator++() {
        while (Index < (s64) Parent->Reserved) {
            Index++;
            if (Index == Parent->Reserved) break;
            if (Parent->_OccupancyMask && Parent->_OccupancyMask[Index]) {
                break;
            }
        }
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
        return {Parent->_Keys + Index, Parent->_Values + Index};
    }

    template <bool NotConst = !Const>
    enable_if_t<!NotConst, pair<const Key *, const Value *>> operator*() const {
        return {Parent->_Keys + Index, Parent->_Values + Index};
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

// :ExplicitDeclareIsPod
template <typename K, typename V>
struct is_pod<table<K, V>> : public true_t {};
template <typename K, typename V>
struct is_pod<table_iterator<K, V, true>> : public true_t {};
template <typename K, typename V>
struct is_pod<table_iterator<K, V, false>> : public true_t {};
template <typename K, typename V>
struct is_pod<const table<K, V>> : public true_t {};
template <typename K, typename V>
struct is_pod<const table_iterator<K, V, true>> : public true_t {};
template <typename K, typename V>
struct is_pod<const table_iterator<K, V, false>> : public true_t {};
template <typename K, typename V>
struct is_pod<const volatile table<K, V>> : public true_t {};
template <typename K, typename V>
struct is_pod<const volatile table_iterator<K, V, true>> : public true_t {};
template <typename K, typename V>
struct is_pod<const volatile table_iterator<K, V, false>> : public true_t {};

template <typename K, typename V>
table<K, V> *clone(table<K, V> *dest, const table<K, V> &src) {
    *dest = {};
    for (auto [k, v] : src) {
        dest->put(k, v);
    }
    return dest;
}

template <typename K, typename V>
table<K, V> *move(table<K, V> *dest, table<K, V> *src) {
    assert(src->is_owner());

    dest->release();
    *dest = *src;

    // Transfer ownership
    change_owner(src->_OccupancyMask, dest);
    change_owner(dest->_OccupancyMask, dest);
    return dest;
}

LSTD_END_NAMESPACE
