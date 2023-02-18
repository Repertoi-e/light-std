module;

#include "../common.h"

export module lstd.hash_table;

export import lstd.memory;
export import lstd.hash;

LSTD_BEGIN_NAMESPACE

// I hate C++. We can't just define this inside hash_table and use them in function signatures...
template <typename HashTableT>
using key_t = HashTableT::K;

template <typename HashTableT>
using value_t = HashTableT::V;

//
// This hash table stores all entries in a contiguous array, for good performance when looking up things. Some tables
// work by storing linked lists of entries, but that can lead to many more cache misses.
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
// This means that 4 values of the hash range have double probabilty of collisions, but it's a small price to pay.
//
// We can't use keys/values to indicate whether slots are empty or removed, because they can be of arbitrary type.
//
// This is a simple implementation, but still fast because we try to keep only 70% of the table full at any time.
// If you are not memory constrained, just make the table bigger and it will get faster.
// Most of the queries are satisfied in the first found slot. The hashing function is also very important for this.
//
// Because of that, storing entries compactly (hash, key, value) is best, because you always take exactly one cache miss per query.
// Using multiple arrays that store data in the form hash-hash-hash, key-key-key leads to way more constant number of cache misses.
//
export {
	template <typename K_, typename V_>
	struct hash_table {
		static const s64 FIRST_VALID_HASH = 2;

		static const s64 MINIMUM_SIZE = 32;
		static const s64 LOAD_FACTOR_PERCENT = 70;

		using K = K_;
		using V = V_;

		struct entry {
			u64 Hash;
			K Key;
			V Value;
		};
		array<entry> Entries;

		s64 Count = 0;  // Number of slots in use
		s64 SlotsFilled = 0;  // Number of slots that can't be used (valid + removed items)
		s64 Allocated = 0;  // Number of slots allocated in total, @Cleanup

		// You can iterate over the table like this:
		//
		//      for (auto [key, value] : table) {
		//          ...
		//      }
		//
	};
}

// is_same_template wouldn't work because hash_table contains a bool (and no type) as a third template parameter.
// At this point I hate C++
template <typename>
const bool is_hash_table = false;

template <typename K, typename V>
const bool is_hash_table<hash_table<K, V>> = true;

template <typename T>
concept any_hash_table = is_hash_table<T>;

export {
	template <any_hash_table T>
	struct key_value_pair {
		key_t<T> *Key;
		value_t<T> *Value;
	};

	// Reserves space equal to the next power of two bigger than _size_, starting at _MINIMUM_SIZE_.
	//
	// Allocates a buffer if the hash table doesn't already point to allocated memory (using the Context's allocator).
	//
	// You don't need to call this before using the hash table.
	// The first time an element is added to the hash table, it reserves with _MINIMUM_SIZE_ and no specified alignment.
	// You can call this before using the hash table to initialize the arrays with a custom alignment (if that's required).
	void resize(any_hash_table auto ref table, s64 slotsToAllocate, u32 alignment = 0) {
		if (slotsToAllocate < table.Allocated) return;

		s64 target = max<s64>(ceil_pow_of_2(slotsToAllocate), table.MINIMUM_SIZE);

		auto oldEntries = table.Entries;

		if (!table.Allocated) {
			reserve(table.Entries, target);
		}
		else {
			reserve(table.Entries, target);
			table.Entries.Count = 0;
		}

		For(range(target)) {
			(table.Entries.Data + it)->Hash = 0;
		}

		// Add the old items
		For_as(it_index, range(table.Allocated)) {
			auto it = oldEntries.Data + it_index;
			if (it->Hash >= table.FIRST_VALID_HASH) add_prehashed(table, it->Hash, it->Key, it->Value);
		}

		table.Allocated = target;

		if (oldEntries.Count) free(oldEntries);
	}

	// Free any memory allocated by this object and reset count
	void free(any_hash_table auto ref table) {
		if (table.Allocated) free(table.Entries);
		table.Allocated = 0;
		table.Count = 0;
		table.SlotsFilled = 0;
	}

	// Don't free the hash table, just destroy contents and reset count
	void reset(any_hash_table auto ref table) {
		For(range(table.Allocated)) {
			(table.Entries.Data + it)->Hash = 0;
		}
		table.Count = 0;
		table.SlotsFilled = 0;
	}

	template <typename T>
	bool compare_equals(T no_copy a, T no_copy b) {
		if constexpr (is_same<T, string>) {
			return strings_match(a, b);
		} else {
			return a == b;
		}
	}

	// Looks for key in the hash table using the given hash
	template <any_hash_table T>
	key_value_pair<T> search_prehashed(T ref table, u64 hash, key_t<T> no_copy key) {
		if (!table.Count) return { null, null };

		s64 index = hash & table.Allocated - 1;
		For(range(table.Allocated)) {
			auto it = table.Entries.Data + index;
			if (it->Hash == hash && compare_equals(it->Key, key)) return { &it->Key, &it->Value };

			++index;
			if (index >= table.Allocated) index = 0;
		}
		return { null, null };
	}

	template <any_hash_table T>
	auto search(T ref table, key_t<T> no_copy key) {
		return search_prehashed(table, get_hash(key), key);
	}

	// Returns pointers to the added key and value.
	template <any_hash_table T>
	key_value_pair<T> add_prehashed(T ref table, u64 hash, key_t<T> no_copy key, value_t<T> no_copy value) {
		static_assert(table.LOAD_FACTOR_PERCENT < 100);  // 100 percent will cause infinite loop

		// The + 1 here handles the case when the hash table size is 1 and you add the first item.
		if ((table.SlotsFilled + 1) * 100 >= table.Allocated * table.LOAD_FACTOR_PERCENT) resize(table, table.SlotsFilled * 2);  // Double size

		assert(table.SlotsFilled < table.Allocated);

		if (hash < table.FIRST_VALID_HASH) hash += table.FIRST_VALID_HASH;

		s64 index = hash & table.Allocated - 1;
		while ((table.Entries.Data + index)->Hash) {
			++index;
			if (index >= table.Allocated) index = 0;
		}

		++table.Count;
		++table.SlotsFilled;

		auto* entry = table.Entries.Data + index;
		*entry = { hash, key, value };
		return { &entry->Key, &entry->Value };
	}

	template <any_hash_table T>
	key_value_pair<T> add(T ref table, key_t<T> no_copy key, value_t<T> no_copy value) {
		return add_prehashed(table, get_hash(key), key, value);
	}

	template <any_hash_table T>
	key_value_pair<T> set_prehashed(T ref table, u64 hash, key_t<T> no_copy key, value_t<T> no_copy value) {
		auto [kp, vp] = search_prehashed(table, hash, key);
		if (vp) {
			*vp = value;
			return { kp, vp };
		}
		return add_prehashed(table, hash, key, value);
	}

	template <any_hash_table T>
	key_value_pair<T> set(T ref table, key_t<T> no_copy key, value_t<T> no_copy value) {
		return set_prehashed(table, get_hash(key), key, value);
	}

	// Returns true if the key was found and removed.
	template <any_hash_table T>
	bool remove_prehashed(T ref table, u64 hash, key_t<T> no_copy key) {
		auto [kp, vp] = search_prehashed(table, hash, key);
		if (vp) {
			s64 index = vp - table.Values;
			table.Hashes[index] = 1;
			return true;
		}
		return false;
	}

	// Returns true if the key was found and removed.
	template <any_hash_table T>
	bool remove(T ref table, key_t<T> no_copy key) {
		return remove_prehashed(table, get_hash(key), key);
	}

	// Returns true if the hash table has the given key.
	template <any_hash_table T>
	bool has(T ref table, key_t<T> no_copy key) { return search(table, key).Key != null; }

	// Returns true if the hash table has the given key.
	template <any_hash_table T>
	bool has_prehashed(T ref table, u64 hash, key_t<T> no_copy key) { return search_prehashed(table, hash, key) != null; }

	template <any_hash_table T>
	bool operator==(T ref t, T ref u) {
		if (t.Entries.Count != u.Entries.Count) return false;

		for (auto [k, v] : t) {
			if (!has(u, *k)) return false;
			if (*v != *search(u, *k).Value) return false;
		}
		return true;
	}

	template <any_hash_table T>
	bool operator!=(T ref t, T ref u) { return !(t == u); }

	template <any_hash_table T>
	T clone(T ref src) {
		T table;
		for (auto [k, v] : src) add(table, *k, *v);
		return table;
	}

	template <any_hash_table T>
	struct hash_table_iterator {
		using hash_table_t = T;

		hash_table_t ref Table;
		s64 Index;

		hash_table_iterator(T ref table, s64 index = 0) : Table(table), Index(index) { skip_empty_slots(); }
		hash_table_iterator& operator++() { return ++Index, skip_empty_slots(), * this; }

		hash_table_iterator operator++(s32) {
			hash_table_iterator pre = *this;
			return ++*this, pre;
		}

		bool operator==(hash_table_iterator other) const { return &Table == &other.Table && Index == other.Index; }
		bool operator!=(hash_table_iterator other) const { return !(*this == other); }

		key_value_pair<hash_table_t> operator*() {
			auto* entry = Table.Entries.Data + Index;
			return { &entry->Key, &entry->Value };
		}

		void skip_empty_slots() {
			for (; Index < Table.Allocated; ++Index) {
				if ((Table.Entries.Data + Index)->Hash >= Table.FIRST_VALID_HASH) break;
			}
		}
	};

	auto begin(any_hash_table auto ref table) { return hash_table_iterator(table); }
	auto end(any_hash_table auto ref table) { return hash_table_iterator(table, table.Allocated); }
}

LSTD_END_NAMESPACE

