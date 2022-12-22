module;

#include "../common.h"

export module lstd.string;

export import lstd.delegate;
export import lstd.stack_array;
export import lstd.c_string_utf8;

LSTD_BEGIN_NAMESPACE

// @TODO: Provide a _string_utf8_validate()_.
// @TODO: Provide a _string_utf8_normalize()_.

//
// This is a string object with text operations on it (assuming valid encoded utf-8),
// that is not null-terminated, but works like an array. It's a basic wrapper around 
// contiguous memory, it contains a char pointer and a size, so any binary file can be 
// read to it without problem. 
// 
// Functions on this object allow negative reversed indexing which begins at
// the end of the array, so -1 is the last code point -2 the one before that, etc. (Python-style)
//
// By default it works like a view, a string doesn't own it's memory unless you explicitly
// treat it like that. We have a very fluid philosophy of containers and ownership. 
// We don't implement copy constructors or destructors, which means that the programmer 
// is totally in control of how the memory gets managed. (See :TypePolicy in "common.h")
// 
// That also means that substrings don't allocate memory, 
// they are just a new data pointer and count.
// 
// In order to get a deep copy of a string use clone(). 
// 
// This object being just two 64 bit integers can be cheaply and safely passed 
// to functions by value without performance concerns and indirection.
// (Remember that the array doesn't "own" it's buffer, so no copy happens)
//
export {
	struct string;
	
	s64 length(string no_copy s);
	void set(string ref s, s64 index, code_point cp);
	code_point get(string str, s64 index);
	void check_debug_memory(string no_copy s);

	struct string {
		char *Data = null;
		s64 Count = 0;
		s64 Allocated = 0;

		// We can't treat it as array-like, because for strings
		// we work with indices which are code point based (and not char based).
		static const bool TREAT_AS_ARRAY_LIKE = false;

		string() {}

		// This constructs a view (use make_string to copy)
		string(const char *str) : Data((char *)str), Count(c_string_length(str)) {}
		
		// This constructs a view (use make_string to copy)
		string(const char8_t* str) : Data((char*)str), Count(c_string_length(str)) {}

		// This constructs a view (use make_string to copy)
		string(const char *str, s64 count) : Data((char *)str), Count(count) {}

		// A special structure which is returned from string_get(),
		// which allows to see the code point at a specific index, but also update it.
		struct code_point_ref {
			string ref String;
			s64 Index;

			code_point_ref(string ref s, s64 index) : String(s) {
				Index = translate_negative_index(index, length(s));
			}

			code_point_ref &operator=(code_point other) {
				set(String, Index, other);
				return *this;
			}

			operator code_point() const { return utf8_decode_cp(utf8_get_pointer_to_cp_at_translated_index(String.Data, length(String), Index)); }
		};

		//
		// for operator[] we return a special structure that allows 
		// assigning a new code point.
		// 
		// This is legal to do:
		//
		//      string a = "Hello";
		//      a[0] = u8'Л';
		//      // a is now "Лello" and contains a two byte code point in the beginning,
		//      // an allocation was possibly made and memory was moved around.
		//      
		// Indexing is code point based, so:
		//      a[1] = u8'Ли';
		//      // a is now "Лиllo", despite "Л" being two bytes 
		//      // a[1] referred to the second code point.
		//
		//
		code_point_ref operator[](s64 index) { return code_point_ref(*this, index); }
		code_point operator[](s64 index) const {
			return utf8_decode_cp(utf8_get_pointer_to_cp_at_translated_index(Data, length(*this), index));
		}
	};

	void reserve(string ref s, s64 n = -1, allocator alloc = {}) {
		if (n <= 0) {
			n = max(s.Count, 8);
		}
		assert(n >= 1);

		using T = remove_pointer_t<decltype(s.Data)>;

		auto* oldData = s.Data;
		if (s.Allocated) {
			s.Data = realloc(s.Data, { .NewCount = n });
		}
		else {
			// Not our job to free oldData since we don't own it.
			// For subsequent reserves we go through the   realloc   branch above,
			// which properly manages to free the old data (if it couldn't reallocate 
			// in place, that is).
			s.Data = malloc<T>({ .Count = n, .Alloc = alloc }); // If alloc is null we use theContext's allocator
			if (oldData) memcpy(s.Data, oldData, s.Count * sizeof(T));
		}

		s.Allocated = n;
	}

	void check_debug_memory(string no_copy s) {
		assert(s.Allocated);
#if defined DEBUG_MEMORY
		//
		// If you assert here, there are two possible reasons:
		// 
		// 1. Attempting to modify a string which is a view
		// and _Data_ wasn't dynamically allocated. This may
		// also happen if an string points to a string from 
		// the text table of the executable.
		//
		// Make sure you call  reserve()  beforehand
		// to copy the contents and make a string which owns
		// memory.
		//
		// 2. Attempting to modify an string from another thread...
		// Caution! This container is not thread-safe!
		//
		assert(debug_memory_list_contains((allocation_header*)s.Data - 1));
#endif
	}

	void free(string ref s) { 
		free(s.Data); 
		s.Count = s.Allocated = 0; 
	}

	// This is <= Count
	s64 length(string no_copy s) { return utf8_length(s.Data, s.Count); }

	// Doesn't allocate memory, strings in this library are not null-terminated.
	// We allow negative reversed indexing which begins at the end of the string,
	// so -1 is the last code point, -2 is the one before that, etc. (Python-style)
	string slice(string s, s64 begin, s64 end);

	//
	// Utilities to convert to c-style strings.
	// Functions for conversion between utf-8, utf-16 and utf-32 are provided in lstd.c_string_utf8
	//

	// Allocates a buffer, copies the string's contents and also appends a zero terminator.
	// Uses the Context's current allocator. The caller is responsible for freeing.
	mark_as_leak char *to_c_string(string s, allocator alloc = {});

	// Allocates a buffer, copies the string's contents and also appends a zero terminator.
	// Uses the temporary allocator.
	//
	// Implemented in lstd.context because otherwise we import in circle.
	char* to_c_string_temp(string s);

	// Returns the code point index (or -1) if not found.
	s64 search(string str, delegate<bool(code_point)> predicate, search_options options = {});

	// Returns the code point index (or -1) if not found.
	s64 search(string str, code_point search, search_options options = {});

	// Returns the code point index (or -1) if not found.
	s64 search(string str, string search, search_options options = {});

	bool has(string str, code_point cp);
	bool has(string str, string s);

	//
	// BIG NOTE: To check equality (with operator == which is defined in array_like.cpp)
	// we check the bytes. However that doesn't always work for Unicode.
	// Some strings which have different representation might be considered equal.
	//    e.g. the character é can be represented either as 'é' or as '´' 
	//         combined with 'e' (two separate characters).
	// Note that UTF-8 specifically requires the shortest-possible encoding for characters,
	// but you still have to be careful (some outputs might not necessarily conform to that).
	// 
	// @TODO @Robustness We should provide a string normalization function to deal with this
	//

	// Compares two utf-8 encoded strings and returns the index
	// of the code point at which they are different or _-1_ if they are the same.
	s64 compare(string s, string other);

	// Compares two utf-8 encoded strings while ignoring case and returns the index
	// of the code point at which they are different or _-1_ if they are the same.
	s64 compare_ignore_case(string s, string other);

	// Compares two utf-8 encoded strings lexicographically and returns:
	//  -1 if _a_ is before _b_
	//   0 if a == b
	//   1 if _b_ is before _a_
	s32 compare_lexicographically(string a, string b);

	// Compares two utf-8 encoded strings lexicographically while ignoring case and returns:
	//  -1 if _a_ is before _b_
	//   0 if a == b
	//   1 if _b_ is before _a_
	s32 compare_lexicographically_ignore_case(string a, string b);

	s32 strings_match(string a, string b) { return compare(a, b) == -1; }
	s32 strings_match_ignore_case(string a, string b) { return compare_ignore_case(a, b) == -1; }

	// Returns true if _s_ begins with _str_
	bool match_beginning(string s, string str) {
		if (str.Count > s.Count) return false;
		return memcmp(s.Data, str.Data, str.Count) == 0;
	}

	// Returns true if _s_ ends with _str_
	bool match_end(string s, string str) {
		if (str.Count > s.Count) return false;
		return memcmp(s.Data + s.Count - str.Count, str.Data, str.Count) == 0;
	}

	// Returns a substring with white space removed at the start
	string trim_start(string s) {
		auto p = [](code_point cp) { return !has(" \n\r\t\v\f", cp); };;
		s64 start = search(s, &p);
		return slice(s, start, length(s));
	}

	// Returns a substring with white space removed at the end
	string trim_end(string s) {
		auto p = [](code_point cp) { return !has(" \n\r\t\v\f", cp); };;
		s64 end = search(s, &p, search_options{ .Start = -1, .Reversed = true }) + 1;
		return slice(s, 0, end);
	}

	// Returns a substring with white space removed from both sides
	string trim(string s) { return trim_end(trim_start(s)); }

	// Changes the code point at _index_ to a new one. 
	// Since utf-8 code points are not the same byte count,
	// this may need to reorder stuff and expand the string.
	// So we assert that the string is dynamically allocated.
	// You can use the next method in order to get finer control.
	void set(string ref s, s64 index, code_point cp);

	void maybe_grow(string ref s, s64 fit) {
		check_debug_memory(s);

		s64 space = s.Allocated;

		if (s.Count + fit <= space) return;

		s64 target = max(ceil_pow_of_2(s.Count + fit + 1), 8);
		reserve(s, target);
	}

	void insert_at_index(string ref s, s64 index, const char *str, s64 size) {
		maybe_grow(s, size);
		
		index = translate_negative_index(index, length(s), true);
		auto *t = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, index);

		s64 offset = translate_negative_index(t - s.Data, s.Count, true);
		auto* where = s.Data + offset;
		if (offset < s.Count) {
			memcpy(where + size, where, (s.Count - offset) * sizeof(*where));
		}
		memcpy(where, str, size * sizeof(*where));
		s.Count += size;
	}

	void insert_at_index(string ref s, s64 index, string str) { insert_at_index(s, index, str.Data, str.Count); }

	void insert_at_index(string ref s, s64 index, code_point cp) {
		char encodedCp[4];
		utf8_encode_cp(encodedCp, cp);
		insert_at_index(s, index, encodedCp, utf8_get_size_of_cp(cp));
	}

	void add(string ref s, const char *ptr, s64 size) { insert_at_index(s, length(s), ptr, size); }
	void add(string ref s, string b) { insert_at_index(s, length(s), b.Data, b.Count); }
	void add(string ref s, code_point cp) { insert_at_index(s, length(s), cp); }

	auto &operator+=(string ref s, code_point cp) {
		add(s, cp);
		return s;
	}

	auto& operator+=(string ref s, string str) {
		add(s, str);
		return s;
	}

	auto &operator+=(string ref s, const char *str) {
		add(s, string(str));
		return s;
	}

	// Remove the first occurrence of a code point.
	// Returns true on success (false if _cp_ was not found in the string).
	bool remove(string ref s, code_point cp);

	// Remove code point at specified index.
	void remove_at_index(string ref s, s64 index);

	// Remove a range of code points. [begin, end)
	void remove_range(string ref s, s64 begin, s64 end);

	void remove_all(string ref s, code_point search);
	void remove_all(string ref s, string search);
	void replace_all(string ref s, code_point search, code_point replace);
	void replace_all(string ref s, code_point search, string replace);
	void replace_all(string ref s, string search, code_point replace);
	void replace_all(string ref s, string search, string replace);

	// Returns a deep copy of _str_ and _count_
	mark_as_leak string make_string(const char *str, s64 count) {
		string result;
		reserve(result, count);
		add(result, str, count);
		return result;
	}

	// Returns a deep copy of _str_
	mark_as_leak string make_string(const char *str) {
		return make_string(str, c_string_length(str));
	}

	// Returns a deep copy of _src_
	mark_as_leak string clone(string no_copy src) {
		return make_string(src.Data, src.Count);
	}

	// This iterator is to make range based for loops work.
	template <bool Const>
	struct string_iterator {
		using string_t = select_t<Const, const string, string>;

		string_t ref String;
		s64 Index;

		string_iterator(string_t ref s, s64 index = 0) : String(s), Index(index) {}

		string_iterator &operator++() {
			Index += 1;
			return *this;
		}

		string_iterator operator++(s32) {
			string_iterator temp = *this;
			++(*this);
			return temp;
		}

		auto operator==(string_iterator other) const { return &String == &other.String && Index == other.Index; }
		auto operator!=(string_iterator other) const { return !(*this == other); }

		auto operator*() { return String[Index]; }
	};

	auto begin(string ref str) { return string_iterator<false>(str, 0); }
	auto begin(string no_copy str) { return string_iterator<true>(str, 0); }
	auto end(string ref str) { return string_iterator<false>(str, length(str)); }
	auto end(string no_copy str) { return string_iterator<true>(str, length(str)); }
}

code_point get(string str, s64 index) {
	if (index < 0) {
		// @Speed... should we cache this in _string_?
		// We need to calculate the total length (in code points)
		// in order for the negative index to be converted properly.
		s64 len = length(str);
		index = translate_negative_index(index, len);

		// If LSTD_ARRAY_BOUNDS_CHECK is defined:
		// _utf8_get_pointer_to_cp_at_translated_index()_ also checks for out of bounds but we are sure the index is valid
		// (since translate_index also checks for out of bounds) so we can get away with the unsafe version here.
		auto *s = str.Data;
		For(range(index)) s += utf8_get_size_of_cp(s);
		return utf8_decode_cp(s);
	} else {
		auto *s = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, index);
		return utf8_decode_cp(s);
	}
}

string slice(string str, s64 begin, s64 end) {
	s64 len = length(str);
	s64 beginIndex = translate_negative_index(begin, len, true);
	s64 endIndex = translate_negative_index(end, len, true);

	const char *beginPtr = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, beginIndex);
	const char *endPtr = beginPtr;

	// @Speed
	For(range(beginIndex, endIndex)) endPtr += utf8_get_size_of_cp(endPtr);

	return string((char *)beginPtr, (s64)(endPtr - beginPtr));
}

s64 search(string str, delegate<bool(code_point)> predicate, search_options options) {
	if (!str.Data || str.Count == 0) return -1;
	s64 len = length(str);
	options.Start = translate_negative_index(options.Start, len, true);
	For(range(options.Start, options.Reversed ? -1 : len, options.Reversed ? -1 : 1)) if (predicate(get(str, it))) return it;
	return -1;
}

s64 search(string str, code_point search, search_options options) {
	if (!str.Data || str.Count == 0) return -1;
	s64 len = length(str);
	options.Start = translate_negative_index(options.Start, len, true);
	For(range(options.Start, options.Reversed ? -1 : len, options.Reversed ? -1 : 1)) if (get(str, it) == search) return it;
	return -1;
}

s64 search(string str, string search, search_options options) {
	if (!str.Data || str.Count == 0) return -1;
	if (!search.Data || search.Count == 0) return -1;

	s64 len = length(str);
	options.Start = translate_negative_index(options.Start, len, true);

	s64 searchLength = length(search);

	For(range(options.Start, options.Reversed ? -1 : len, options.Reversed ? -1 : 1)) {
		s64 progress = 0;
		for (s64 s = it; progress != searchLength; ++s, ++progress) {
			if (!(get(str, s) == get(search, progress))) break;
		}
		if (progress == searchLength) return it;
	}
	return -1;
}

bool has(string str, string s) {
	return search(str, s) != -1;
}

bool has(string str, code_point cp) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, cp);
	return search(str, string(encodedCp, utf8_get_size_of_cp(cp))) != -1;
}

s64 compare(string s, string other) {
	if (!s.Count && !other.Count) return -1;
	if (!s.Count || !other.Count) return 0;

	auto *p1 = s.Data, *p2 = other.Data;
	auto *e1 = p1 + s.Count, *e2 = p2 + other.Count;

	s64 index = 0;
	while (utf8_decode_cp(p1) == utf8_decode_cp(p2)) {
		p1 += utf8_get_size_of_cp(p1);
		p2 += utf8_get_size_of_cp(p2);
		if (p1 == e1 && p2 == e2) return -1;
		if (p1 == e1 || p2 == e2) return index;
		++index;
	}
	return index;
}

s64 compare_ignore_case(string s, string other) {
	if (!s.Count && !other.Count) return -1;
	if (!s.Count || !other.Count) return 0;

	auto *p1 = s.Data, *p2 = other.Data;
	auto *e1 = p1 + s.Count, *e2 = p2 + other.Count;

	s64 index = 0;
	while (to_lower(utf8_decode_cp(p1)) == to_lower(utf8_decode_cp(p2))) {
		p1 += utf8_get_size_of_cp(p1);
		p2 += utf8_get_size_of_cp(p2);
		if (p1 == e1 && p2 == e2) return -1;
		if (p1 == e1 || p2 == e2) return index;
		++index;
	}
	return index;
}

s32 compare_lexicographically(string a, string b) {
	if (!a.Count && !b.Count) return 0;
	if (!a.Count) return -1;
	if (!b.Count) return 1;

	auto *p1 = a.Data, *p2 = b.Data;
	auto *e1 = p1 + a.Count, *e2 = p2 + b.Count;

	s64 index = 0;
	while (utf8_decode_cp(p1) == utf8_decode_cp(p2)) {
		p1 += utf8_get_size_of_cp(p1);
		p2 += utf8_get_size_of_cp(p2);
		if (p1 == e1 && p2 == e2) return 0;
		if (p1 == e1) return -1;
		if (p2 == e2) return 1;
		++index;
	}
	return ((s64)utf8_decode_cp(p1) - (s64)utf8_decode_cp(p2)) < 0 ? -1 : 1;
}

s32 compare_lexicographically_ignore_case(string a, string b) {
	if (!a.Count && !b.Count) return 0;
	if (!a.Count) return -1;
	if (!b.Count) return 1;

	auto *p1 = a.Data, *p2 = b.Data;
	auto *e1 = p1 + a.Count, *e2 = p2 + b.Count;

	s64 index = 0;
	while (to_lower(utf8_decode_cp(p1)) == to_lower(utf8_decode_cp(p2))) {
		p1 += utf8_get_size_of_cp(p1);
		p2 += utf8_get_size_of_cp(p2);
		if (p1 == e1 && p2 == e2) return 0;
		if (p1 == e1) return -1;
		if (p2 == e2) return 1;
		++index;
	}
	return ((s64)to_lower(utf8_decode_cp(p1)) - (s64)to_lower(utf8_decode_cp(p2))) < 0 ? -1 : 1;
}

void replace_range(string ref str, s64 begin, s64 end, string replace) {

	s64 targetBegin = translate_negative_index(begin, str.Count);
	s64 targetEnd = translate_negative_index(end, str.Count, true);

	s64 whereSize = targetEnd - targetBegin;

	s64 diff = replace.Count - whereSize;

	if (diff > 0) {
		maybe_grow(str, diff);
	}

	auto where = str.Data + targetBegin;

	// Make space for the new elements
	memcpy(where + replace.Count, where + whereSize, (str.Count - targetBegin - whereSize) * sizeof(*where));

	// Copy replace elements
	memcpy(where, replace.Data, replace.Count * sizeof(*where));

	str.Count += diff;
}

void set(string ref str, s64 index, code_point cp) {
	check_debug_memory(str);

	index = translate_negative_index(index, length(str));

	const char *target = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, index);

	char encodedCp[4];
	utf8_encode_cp(encodedCp, cp);

	string replace = string(encodedCp, utf8_get_size_of_cp(cp));
	s64 begin = target - str.Data;
	s64 end = target - str.Data + utf8_get_size_of_cp(target);
	replace_range(str, begin, end, replace);
}

mark_as_leak char *to_c_string(string s, allocator alloc) {
	char *result = malloc<char>({ .Count = s.Count + 1, .Alloc = alloc });
	memcpy(result, s.Data, s.Count);
	result[s.Count] = '\0';
	return result;
}

bool remove(string ref s, code_point cp) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, cp);

	s64 index = search(s, string(encodedCp, utf8_get_size_of_cp(cp)));
	if (index == -1) return false;

	remove_range(s, index, index + utf8_get_size_of_cp(cp));

	return true;
}

void remove_at_index(string ref s, s64 index) {
	index = translate_negative_index(index, length(s));

	auto *t = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, index);

	s64 b = t - s.Data;
	remove_range(s, b, b + utf8_get_size_of_cp(t));
}

void remove_range(string ref s, s64 begin, s64 end) {
	check_debug_memory(s);
	
	s64 len = length(s);

	begin = translate_negative_index(begin, len);
	end = translate_negative_index(end, len, true);

	auto *tbp = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, begin);
	auto *tep = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, end);

	begin = (s64)(tbp - s.Data);
	end = (s64)(tep - s.Data);

	s64 tp = translate_negative_index(begin, s.Count);
	s64 te = translate_negative_index(end, s.Count, true);

	auto where = s.Data + tp;
	auto whereEnd = s.Data + te;

	s64 elementCount = whereEnd - where;
	memcpy(where, whereEnd, (s.Count - tp - elementCount) * sizeof(*where));
	s.Count -= elementCount;
}

void replace_all(string ref s, string search, string replace) {
	// @CutAndPaste from array-like's replace_all.
	// @Volatile
	check_debug_memory(s);

	if (!s.Data || !s.Count) return;

	assert(search.Data && search.Count);
	if (replace.Count) assert(replace.Data);

	if (search.Count == replace.Count) {
		// This case we can handle relatively fast.
		// @Speed Improve by using bit hacks for the case when the elements are less than a pointer size?
		auto* p = s.Data;
		auto* e = s.Data + s.Count;
		while (p != e) {
			// @Speed We can do simply memcmp for scalar types and types that don't haveoverloaded ==.
			if (*p == search[0]) {
				auto* n = p;
				auto* sp = search.Data;
				auto* se = search.Data + search.Count;
				while (n != e && sp != se) {
					// Require only operator == to be defined (and not !=).
					if (!(*n == *sp)) break;
					++n, ++sp;
				}

				if (sp == se) {
					// Match found
					memcpy(p, replace.Data, replace.Count * sizeof(*p));
					p += replace.Count;
				}
				else {
					++p;
				}
			}
			else {
				++p;
			}
		}
	}
	else {
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
		while (i < s.Count && (i = ::search(s, search, search_options{ .Start = i })) != -1) {
			replace_range(s, i, i + search.Count, replace);  // @Speed Slow and dumb version for now
			i += replace.Count;
		}
	}
}

void replace_all(string ref s, code_point search, code_point replace) {
	char encodedOld[4];
	utf8_encode_cp(encodedOld, search);

	char encodedNew[4];
	utf8_encode_cp(encodedNew, replace);

	replace_all(s, string(encodedOld, utf8_get_size_of_cp(encodedOld)), string(encodedNew, utf8_get_size_of_cp(encodedNew)));
}

void remove_all(string ref s, code_point search) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, search);

	replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), string(""));
}

void remove_all(string ref s, string search) { replace_all(s, search, string("")); }

void replace_all(string ref s, code_point search, string replace) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, search);

	replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), replace);
}

void replace_all(string ref s, string search, code_point replace) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, replace);

	replace_all(s, search, string(encodedCp, utf8_get_size_of_cp(encodedCp)));
}

LSTD_END_NAMESPACE
