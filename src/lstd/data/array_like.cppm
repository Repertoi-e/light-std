module;

#include "../common.h"

export module lstd.array_like;

export import lstd.qsort;
export import lstd.memory;
export import lstd.delegate;

//
// :CodeReusability: This file implements:
//  * search, has, compare, compare_lexicographically, <=>
//  * reserve, maybe_grow, insert_at_index, insert_at_index, add, add, remove_ordered_at_index, remove_unordered_at_index, remove_ordered, remove_unordered, remove_range, replace_range, remove_all, replace_all
//
// ... for structures that have members Data and Count (and Allocated for dynamic arrays) 
// - we call these array-likes.
//
// Your custom types (which aren't explicitly flagged) will also automatically get this treatment.
// You can explicitly disable this with a member "static const bool TREAT_AS_ARRAY_LIKE = false;"
//
// e.g. stack_array, array, array_view, string - all have the members Data and Count, 
// so functions are automatically generated to all these types using the definition below.
// They also resolve for different (but compatible) types (e.g. comparing stack_array<u8> 
// and array<u8>) for which otherwise we need a combinatorial amount of code to support.
//
// @TODO: We currently expect the members to be named exactly Data and Count which may 
// conflict with other naming styles. How should we handle this?
//

LSTD_BEGIN_NAMESPACE

template <typename T>
concept does_array_have_flag = requires(T t) {
	{ t.TREAT_AS_ARRAY_LIKE };
};

template <typename T>
constexpr bool should_array_be_treated_as_array_like() {
	if constexpr (does_array_have_flag<T>) {
		return T::TREAT_AS_ARRAY_LIKE;
	}
	return true;
}

template <typename T>
concept has_array_members = requires(T t) {
	{ t.Data };
	{ t.Count };
};

template <typename T>
concept has_dynamic_array_members = requires(T t) {
	{ t.Data };
	{ t.Count };
	{ t.Allocated };
};

template <typename T>
concept is_array_like = should_array_be_treated_as_array_like<T>() && has_array_members<T>;

template <typename T>
concept is_dynamic_array_like = should_array_be_treated_as_array_like<T>() && has_dynamic_array_members<T>;

export {
	// True if the type has _Data_ and _Count_ members (and the optional explicit "TREAT_AS_ARRAY_LIKE" flag is not false).
	template <typename T>
	concept any_array_like = is_array_like<remove_cv_t<T>>;

	// True if the type has _Data_, _Count_, and _Allocated_ members (and the optional explicit "TREAT_AS_ARRAY_LIKE" flag is not false).
	template <typename T>
	concept any_dynamic_array_like = is_dynamic_array_like<remove_cv_t<T>>;

	// This returns the type of the _Data_ member of an array-like object
	template <any_array_like T> using array_data_t = remove_pointer_t<decltype(T::Data)>;

	//
	// This function translates an index that may be negative to an actual index.
	// For example 5 maps to 5
	// but -5 maps to length - 5
	//
	// It is used to support Python-like negative indexing.
	//
	// This function checks if the index is in range if LSTD_ARRAY_BOUNDS_CHECK is defined.
	//    In that case if _toleratePastLast_ is true, index == length is also accepted.
	//    This is useful when you are calculating the end index of an exclusive range
	//    and you don't want to trip an out of bounds assert.
	//
	always_inline s64 translate_negative_index(s64 index, s64 length, bool toleratePastLast = false);

	struct search_options {
		s64 Start = 0; // At which index to begin searching
		bool Reversed = false;
	};

	//
	// @Speed Optimize these functions for scalars (bit hacks and vectorization)
	//
	// 
	// Find the first occurrence of an element which matches the predicate. 
	// Predicate must take a single argument (the current element) and return if it matches.
	template <any_array_like Arr>
	s64 search(Arr no_copy arr, delegate<bool(array_data_t<Arr> no_copy)> predicate, search_options options = {});

	// Find the first occurrence of an element which matches. 
	template <any_array_like Arr>
	s64 search(Arr no_copy arr, array_data_t<Arr> no_copy search, search_options options = {});

	// Find the first occurrence of a subarray, compares elements using ==
	s64 search(any_array_like auto no_copy arr, any_array_like auto no_copy search, search_options options = {});

	bool has(any_array_like auto no_copy arr, auto no_copy item) { return search(arr, item) != -1; }

	// Compares this array to _arr_ and returns the index of the first element that is different.
	// If the arrays are equal, the returned value is -1.
	s64 compare(any_array_like auto no_copy a, any_array_like auto no_copy b);

	// Compares this array to to _arr_ lexicographically.
	// The result is -1 if this array sorts before the other, 0 if they are equal, and +1 otherwise.
	s32 compare_lexicographically(any_array_like auto no_copy a, any_array_like auto no_copy b);

	bool operator==(any_array_like auto no_copy a, any_array_like auto no_copy b) { return compare(a, b) == -1; }
	bool operator!=(any_array_like auto no_copy a, any_array_like auto no_copy b) { return compare(a, b) != -1; }
	bool operator<(any_array_like auto no_copy a, any_array_like auto no_copy b) { return compare_lexicographically(a, b) < 0; }
	bool operator>(any_array_like auto no_copy a, any_array_like auto no_copy b) { return compare_lexicographically(a, b) > 0; }
	bool operator<=(any_array_like auto no_copy a, any_array_like auto no_copy b) { return compare_lexicographically(a, b) <= 0; }
	bool operator>=(any_array_like auto no_copy a, any_array_like auto no_copy b) { return compare_lexicographically(a, b) >= 0; }

	// Doesn't allocate, returns a sub-array of _arr_.
	auto slice(any_array_like auto ref arr, s64 begin, s64 end) {
		s64 beginIndex = translate_negative_index(begin, arr.Count, true);
		s64 endIndex = translate_negative_index(end, arr.Count, true);

		decltype(arr) result;
		result.Data = arr.Data + beginIndex;
		result.Count = arr.Data + endIndex - result.Data;
		return result;
	}

	// Sets the length of allocated storage to at least n. 
	// It will not change the length of the array.
	// 
	// For _n_ you can pass the size of the buffer you want or just leave
	// it at -1 (in which case we allocate Count or a minimum of 1).
	// 
	// When modifying dynamic arrays, the object grows and allocates automatically, 
	// so you can skip managing this, but coming up with a good value for 
	// the initial _n_ can significantly improve performance.
	//
	// In the end, _arr_ is a newly allocated/reallocated array.
	void reserve(any_dynamic_array_like auto ref arr, s64 n = -1, allocator alloc = {});

	// Checks _arr_ if there is space for at least _fit_ new elements.
	// Reserves space in the array if there is not enough. The new size is equal 
	// to the next power of two bigger than (arr.Count + fit), minimum 1.
	//
	// In the end, _arr_ is a newly allocated/reallocated array.
	void maybe_grow(any_dynamic_array_like auto ref arr, s64 fit);

	template <any_dynamic_array_like Arr>
	auto *insert_at_index(Arr ref arr, s64 index, array_data_t<Arr> no_copy element); // Returns pointer in the array to the added element

	template <any_dynamic_array_like Arr>
	auto *insert_at_index(Arr ref arr, s64 index, const array_data_t<Arr> *ptr, s64 size); // Returns pointer in the array to the beginning of added elements 

	template <any_dynamic_array_like Arr, any_array_like Arr2>
		requires(is_same<array_data_t<Arr>, array_data_t<Arr2>>)
	auto *insert_at_index(Arr ref arr, s64 index, Arr2 no_copy arr2) { return insert_at_index(arr, index, arr2.Data, arr2.Count); } // Returns pointer in the array to the beginning of added elements 

	template <any_dynamic_array_like Arr>
	auto *insert_at_index(any_dynamic_array_like auto ref arr, s64 index, initializer_list<array_data_t<Arr>> list) { return insert_at_index(arr, index, list.begin(), list.end() - list.begin()); } // Returns pointer in the array to the beginning of added elements 

	template <any_dynamic_array_like Arr>
	auto *add(Arr ref arr, array_data_t<Arr> no_copy element) { return insert_at_index(arr, arr.Count, element); }

	template <any_dynamic_array_like Arr, any_array_like Arr2>
		requires(is_same<array_data_t<Arr>, array_data_t<Arr2>>)
	auto *add(Arr ref arr, Arr2 no_copy arr2) { return insert_at_index(arr, arr.Count, arr2); }

	template <any_dynamic_array_like Arr>
	auto *add(Arr ref arr, initializer_list<array_data_t<Arr>> list) { return insert_at_index(arr, arr.Count, list.begin(), list.end() - list.begin()); }

	template <any_dynamic_array_like Arr>
	auto *add(Arr ref arr, const array_data_t<Arr> *ptr, s64 size) { return insert_at_index(arr, arr.Count, ptr, size); }

	template <any_dynamic_array_like Arr, any_array_like Arr2>
		requires(is_same<array_data_t<Arr>, array_data_t<Arr2>>)
	auto &operator+=(Arr ref arr, Arr2 no_copy arr2) {
		add(arr, arr2);
		return arr;
	}

	template <any_dynamic_array_like Arr>
	auto &operator+=(Arr ref arr, initializer_list<array_data_t<Arr>> list) {
		add(arr, list);
		return arr;
	}

	// Removes element at specified index and moves following elements back
	void remove_ordered_at_index(any_dynamic_array_like auto ref arr, s64 index);

	// Removes element at specified index and moves the last element to the empty slot.
	// This is faster than remove because it doesn't move everything back
	// but this doesn't keep the order of the elements.
	void remove_unordered_at_index(any_dynamic_array_like auto ref arr, s64 index);

	// Removes first found element and moves following elements back.
	// Returns true on success (false if _element_ was not found in the array).
	template <any_dynamic_array_like Arr>
	bool remove_ordered(Arr ref arr, array_data_t<Arr> no_copy element);

	// Removes first found element and moves the last element to the empty slot.
	// This is faster than remove because it doesn't move everything back
	// but this doesn't keep the order of the elements.
	// Returns true on success (false if _element_ was not found in the array).
	template <any_dynamic_array_like Arr>
	void remove_unordered(Arr ref arr, array_data_t<Arr> no_copy element);

	// Removes a range [begin, end) and moves following elements back
	void remove_range(any_dynamic_array_like auto ref arr, s64 begin, s64 end);

	// Removes a range [begin, end) and inserts _replace_.
	// May allocate and change the count:
	// moves following elements forward/backward if (end - begin) != replace.Count.
	template <any_dynamic_array_like Arr, any_array_like Arr2>
		requires(is_same<array_data_t<Arr>, array_data_t<Arr2>>)
	void replace_range(Arr ref arr, s64 begin, s64 end, Arr2 no_copy replace);

	// Replace all occurrences of _search_ with _replace_
	template <any_dynamic_array_like Arr, any_array_like Arr2, any_array_like Arr3>
		requires(is_same<array_data_t<Arr>, array_data_t<Arr2>> &&is_same<array_data_t<Arr>, array_data_t<Arr3>>)
	void replace_all(Arr ref arr, Arr2 no_copy search, Arr3 no_copy replace);
	
	// Remove all occurrences of _search_
	template <any_dynamic_array_like Arr, any_array_like Arr2>
		requires(is_same<array_data_t<Arr>, array_data_t<Arr2>>)
	void remove_all(Arr ref arr, Arr2 no_copy search) { replace_all(arr, search, {}); }

	// To make range based for loops work.
	auto begin(any_array_like auto ref arr) { return arr.Data; }
	auto begin(any_array_like auto no_copy arr) { return arr.Data; }
	auto end(any_array_like auto ref arr) { return arr.Data + arr.Count; }
	auto end(any_array_like auto no_copy arr) { return arr.Data + arr.Count; }

	void check_debug_memory(any_dynamic_array_like auto no_copy arr);
}

always_inline s64 translate_negative_index(s64 index, s64 length, bool toleratePastLast) {
	if (index < 0) {
		s64 r = length + index;
	#if defined LSTD_ARRAY_BOUNDS_CHECK
		assert(r >= 0 && "Out of bounds");
		assert(r < (toleratePastLast ? length + 1 : length) && "Out of bounds");
	#endif
		return r;
	} else {
	#if LSTD_ARRAY_BOUNDS_CHECK
		assert(index < (toleratePastLast ? length + 1 : length) && "Out of bounds");
	#endif
		return index;
	}
}

template <any_array_like Arr>
s64 search(Arr no_copy arr, delegate<bool(array_data_t<Arr> no_copy)> predicate, search_options options) {
	if (!arr.Data || arr.Count == 0) return -1;
	options.Start = translate_negative_index(options.Start, arr.Count);
	For(range(options.Start, options.Reversed ? -1 : arr.Count, options.Reversed ? -1 : 1)) if (predicate(arr.Data[it])) return it;
	return -1;
}

template <any_array_like Arr>
s64 search(Arr no_copy arr, array_data_t<Arr> no_copy search, search_options options) {
	auto predicate = [&](array_data_t<Arr> no_copy element) { return search == element; };
	return ::search(arr, &predicate, options);
}

s64 search(any_array_like auto no_copy arr, any_array_like auto no_copy search, search_options options) {
	if (!arr.Data || arr.Count == 0) return -1;
	if (!search.Data || search.Count == 0) return -1;
	options.Start = translate_negative_index(options.Start, arr.Count);

	auto searchEnd = search.Data + search.Count;

	For(range(options.Start, options.Reversed ? -1 : arr.Count, options.Reversed ? -1 : 1)) {
		auto progress = search.Data;
		for (auto s = arr.Data + it; progress != searchEnd; ++s, ++progress) {
			if (!(*s == *progress)) break;
		}
		if (progress == searchEnd) return it;
	}
	return -1;
}

s64 compare(any_array_like auto no_copy a, any_array_like auto no_copy b) {
	if (!a.Count && !b.Count) return -1;
	if (!a.Count || !b.Count) return 0;

	auto *p1 = a.Data;
	auto *p2 = b.Data;
	auto *e1 = a.Data + a.Count;
	auto *e2 = b.Data + b.Count;
	while (*p1 == *p2) {
		++p1, ++p2;
		if (p1 == e1 && p2 == e2) return -1;
		if (p1 == e1) return p1 - a.Data;
		if (p2 == e2) return p2 - b.Data;
	}
	return p1 - a.Data;
}

s32 compare_lexicographically(any_array_like auto no_copy a, any_array_like auto no_copy b) {
	if (!a.Count && !b.Count) return 0;
	if (!a.Count) return -1;
	if (!b.Count) return 1;

	auto *p1 = a.Data;
	auto *p2 = b.Data;
	auto *e1 = a.Data + a.Count;
	auto *e2 = b.Data + b.Count;
	while (*p1 == *p2) {
		++p1, ++p2;
		if (p1 == e1 && p2 == e2) return 0;
		if (p1 == e1) return -1;
		if (p2 == e2) return 1;
	}
	return *p1 < *p2 ? -1 : 1;
}

void check_debug_memory(any_dynamic_array_like auto no_copy arr) {
	//
	// If you assert here, there are two possible reasons:
	// 
	// 1. Attempting to modify a array which is a view
	// and _Data_ wasn't dynamically allocated.
	//
	// Make sure you call  reserve()  beforehand
	// to copy the contents and make a array which owns
	// memory.
	//
	// 2. Attempting to modify an array from another thread...
	// Caution! This container is not thread-safe!
	//
	assert(arr.Allocated);
#if defined DEBUG_MEMORY
	assert(debug_memory_list_contains((allocation_header *)arr.Data - 1));
#endif
}

void reserve(any_dynamic_array_like auto ref arr, s64 n, allocator alloc) {
	if (n <= 0) {
		n = max(arr.Count, 8);
	}
	assert(n >= 1);

	using T = remove_pointer_t<decltype(arr.Data)>;

	auto *oldData = arr.Data;
	if (arr.Allocated) {
		arr.Data = realloc(arr.Data, { .NewCount = n });
	} else {
		// Not our job to free oldData since we don't own it.
		// For subsequent reserves we go through the   realloc   branch above,
		// which properly manages to free the old data (if it couldn't reallocate 
		// in place, that is).
		arr.Data = malloc<T>({ .Count = n, .Alloc = alloc }); // If alloc is null we use the Context's allocator
		if (oldData) memcpy(arr.Data, oldData, arr.Count * sizeof(T));
	}
	
	arr.Allocated = n;
}

void maybe_grow(any_dynamic_array_like auto ref arr, s64 fit) {
	check_debug_memory(arr);

	s64 space = arr.Allocated;

	if (arr.Count + fit <= space) return;

	s64 target = max(ceil_pow_of_2(arr.Count + fit + 1), 8);
	reserve(arr, target);
}

template <any_dynamic_array_like Arr>
auto *insert_at_index(Arr ref arr, s64 index, array_data_t<Arr> no_copy element) {
	maybe_grow(arr, 1);

	s64 offset = translate_negative_index(index, arr.Count, true);
	auto *where = arr.Data + offset;
	if (offset < arr.Count) {
		memcpy(where + 1, where, (arr.Count - offset) * sizeof(*where));
	}
	*where = element;
	++arr.Count;
	return where;
}

template <any_dynamic_array_like Arr>
auto *insert_at_index(Arr ref arr, s64 index, const array_data_t<Arr> *ptr, s64 size) {
	maybe_grow(arr, size);

	s64 offset = translate_negative_index(index, arr.Count, true);
	auto *where = arr.Data + offset;
	if (offset < arr.Count) {
		memcpy(where + size, where, (arr.Count - offset) * sizeof(*where));
	}
	memcpy(where, ptr, size * sizeof(*where));
	arr.Count += size;
	return where;
}

void remove_ordered_at_index(any_dynamic_array_like auto ref arr, s64 index) {
	check_debug_memory(arr);

	s64 offset = translate_negative_index(index, arr.Count);

	auto *where = arr.Data + offset;
	memcpy(where, where + 1, (arr.Count - offset - 1) * sizeof(*where));
	--arr.Count;
}

void remove_unordered_at_index(any_dynamic_array_like auto ref arr, s64 index) {
	check_debug_memory(arr);

	s64 offset = translate_negative_index(index, arr.Count);

	auto *where = arr.Data + offset;

	// No need when removing the last element
	if (offset != arr.Count - 1) {
		*where = arr.Data + arr.Count - 1;
	}
	--arr.Count;
}

template <any_dynamic_array_like Arr>
bool remove_ordered(Arr ref arr, array_data_t<Arr> no_copy element) {
	s64 index = search(arr, element);
	if (index == -1) return false;

	remove_ordered_at_index(arr, index);

	return true;
}

template <any_dynamic_array_like Arr>
void remove_unordered(Arr ref arr, array_data_t<Arr> no_copy element) {
	s64 index = search(arr, element);
	if (index == -1) return false;

	remove_unordered_at_index(arr, index);

	return true;
}

void remove_range(any_dynamic_array_like auto ref arr, s64 begin, s64 end) {
	check_debug_memory(arr);

	s64 tp = translate_negative_index(begin, arr.Count);
	s64 te = translate_negative_index(end, arr.Count, true);

	auto where = arr.Data + tp;
	auto whereEnd = arr.Data + te;

	s64 elementCount = whereEnd - where;
	memcpy(where, whereEnd, (arr.Count - tp - elementCount) * sizeof(*where));
	arr.Count -= elementCount;
}

template <any_dynamic_array_like Arr, any_array_like Arr2>
	requires(is_same<array_data_t<Arr>, array_data_t<Arr2>>)
void replace_range(Arr ref arr, s64 begin, s64 end, Arr2 no_copy replace) {
	check_debug_memory(arr);

	s64 targetBegin = translate_negative_index(begin, arr.Count);
	s64 targetEnd = translate_negative_index(end, arr.Count, true);

	s64 whereSize = targetEnd - targetBegin;

	s64 diff = replace.Count - whereSize;

	if (diff > 0) {
		maybe_grow(arr, diff);
	}

	auto where = arr.Data + targetBegin;

	// Make space for the new elements
	memcpy(where + replace.Count, where + whereSize, (arr.Count - targetBegin - whereSize) * sizeof(*where));

	// Copy replace elements
	memcpy(where, replace.Data, replace.Count * sizeof(*where));

	arr.Count += diff;
}

template <any_dynamic_array_like Arr, any_array_like Arr2, any_array_like Arr3>
	requires(is_same<array_data_t<Arr>, array_data_t<Arr2>> &&is_same<array_data_t<Arr>, array_data_t<Arr3>>)
void replace_all(Arr ref arr, Arr2 no_copy search, Arr3 no_copy replace) {
	check_debug_memory(arr);

	if (!arr.Data || !arr.Count) return;

	assert(search.Data && search.Count);
	if (replace.Count) assert(replace.Data);

	if (search.Count == replace.Count) {
		// This case we can handle relatively fast.
		// @Speed Improve by using bit hacks for the case when the elements are less than a pointer size?
		auto *p = arr.Data;
		auto *e = arr.Data + arr.Count;
		while (p != e) {
			// @Speed We can do simply memcmp for scalar types and types that don't haveoverloaded ==.
			if (*p == search[0]) {
				auto *n = p;
				auto *sp = search.Data;
				auto *se = search.Data + search.Count;
				while (n != e && sp != se) {
					// Require only operator == to be defined (and not !=).
					if (!(*n == *sp)) break;
					++n, ++sp;
				}

				if (sp == se) {
					// Match found
					memcpy(p, replace.Data, replace.Count * sizeof(*p));
					p += replace.Count;
				} else {
					++p;
				}
			} else {
				++p;
			}
		}
	} else {
		//
		// @Speed This is the slow and dumb version for now.
		// We can improve performance by either:
		// * Allocating a buffer first which holds the result (space cost increases)
		// * Doing two passes, first one counting the number of occurrences
		//   so we know the offsets for the second pass.
		// Though the second option would only work if search.Count > replace.Count.
		//
		// I think going with the former makes the most sense,
		// however at that point letting the caller write their own routine
		// will probably be better, since we can't for sure know the context
		// and if allocating another (possibly big) array is fine.
		//
		s64 diff = replace.Count - search.Count;

		s64 i = 0;
		while (i < arr.Count && (i = ::search(arr, search, search_options{ .Start = i })) != -1) {
			replace_range(arr, i, i + search.Count, replace);  // @Speed Slow and dumb version for now
			i += replace.Count;
		}
	}
}

LSTD_END_NAMESPACE
