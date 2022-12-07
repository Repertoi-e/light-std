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
	
	constexpr s64 string_length(string no_copy s);
	void string_set(string ref s, s64 index, code_point cp);
	constexpr code_point string_get(string str, s64 index);

	struct string {
		char *Data = null;
		s64 Count = 0;
		s64 Allocated = 0;

		constexpr string() {}

		// This constructs a view (use make_string to copy)
		constexpr string(const char *str) : Data((char *)str), Count(c_string_length(str)) {}
		
		// This constructs a view (use make_string to copy)
		constexpr string(const char *str, s64 count) : Data((char *)str), Count(count) {}

		// A special structure which is returned from string_get(),
		// which allows to see the code point at a specific index, but also update it.
		struct code_point_ref {
			string ref String;
			s64 Index;

			code_point_ref(string ref s, s64 index) : String(s) {
				assert(s.Allocated);
				array_check_debug_memory(s);
				Index = translate_index(index, string_length(s));
			}

			// @TODO: constexpr
			code_point_ref &operator=(code_point other) {
				string_set(String, Index, other);
				return *this;
			}

			operator code_point() const { return utf8_decode_cp(utf8_get_pointer_to_cp_at_translated_index(String.Data, string_length(String), Index)); }
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
			return utf8_decode_cp(utf8_get_pointer_to_cp_at_translated_index(Data, string_length(*this), index));
		}

		constexpr operator bool() const { return Count; }
	};

	void free(string ref s) { 
		free(s.Data); 
		s.Count = s.Allocated = 0; 
	}

	// This is <= Count
	constexpr s64 string_length(string ref s) { return utf8_length(s.Data, s.Count); }

	// Doesn't allocate memory, strings in this library are not null-terminated.
	// We allow negative reversed indexing which begins at the end of the string,
	// so -1 is the last code point, -2 is the one before that, etc. (Python-style)
	constexpr string string_slice(string s, s64 begin, s64 end);

	//
	// Utilities to convert to c-style strings.
	// Functions for conversion between utf-8, utf-16 and utf-32 are provided in lstd.c_string_utf8
	//

	// Allocates a buffer, copies the string's contents and also appends a zero terminator.
	// Uses the Context's current allocator. The caller is responsible for freeing.
	mark_as_leak char *string_to_c_string(string s, allocator alloc = {});

	// Allocates a buffer, copies the string's contents and also appends a zero terminator.
	// Uses the temporary allocator.
	//
	// Implemented in lstd.context because otherwise we import in circle.
	char *string_to_c_string_temp(string s);


	//
	// String searching (contrary to array_search, these indices refer to code points, and not bytes)
	//

	// Returns the code point index (or -1) if not found.
	s64 string_search(string str, delegate<bool(code_point)> predicate, search_options options = {});

	// Returns the code point index (or -1) if not found.
	constexpr s64 string_search(string str, code_point search, search_options options = {});

	// Returns the code point index (or -1) if not found.
	constexpr s64 string_search(string str, string search, search_options options = {});

	constexpr bool string_has(string str, code_point cp);
	constexpr bool string_has(string str, string s);

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
	constexpr s64 string_compare(string s, string other);

	// Compares two utf-8 encoded strings while ignoring case and returns the index
	// of the code point at which they are different or _-1_ if they are the same.
	constexpr s64 string_compare_ignore_case(string s, string other);

	// Compares two utf-8 encoded strings lexicographically and returns:
	//  -1 if _a_ is before _b_
	//   0 if a == b
	//   1 if _b_ is before _a_
	constexpr s32 string_compare_lexicographically(string a, string b);

	// Compares two utf-8 encoded strings lexicographically while ignoring case and returns:
	//  -1 if _a_ is before _b_
	//   0 if a == b
	//   1 if _b_ is before _a_
	constexpr s32 string_compare_lexicographically_ignore_case(string a, string b);

	constexpr s32 strings_match(string a, string b) { return string_compare(a, b) == -1; }
	constexpr s32 strings_match_ignore_case(string a, string b) { return string_compare_ignore_case(a, b) == -1; }

	// Returns true if _s_ begins with _str_
	constexpr bool string_match_beginning(string s, string str) {
		if (str.Count > s.Count) return false;
		return memcmp(s.Data, str.Data, str.Count) == 0;
	}

	// Returns true if _s_ ends with _str_
	constexpr bool string_match_end(string s, string str) {
		if (str.Count > s.Count) return false;
		return memcmp(s.Data + s.Count - str.Count, str.Data, str.Count) == 0;
	}

	// Returns a substring with white space removed at the start
	string string_trim_start(string s) {
		auto p = [](code_point cp) { return !string_has(" \n\r\t\v\f", cp); };;
		s64 start = string_search(s, &p);
		return string_slice(s, start, string_length(s));
	}

	// Returns a substring with white space removed at the end
	string string_trim_end(string s) {
		auto p = [](code_point cp) { return !string_has(" \n\r\t\v\f", cp); };;
		s64 end = string_search(s, &p, search_options{ .Start = -1, .Reversed = true }) + 1;
		return string_slice(s, 0, end);
	}

	// Returns a substring with white space removed from both sides
	string string_trim(string s) { return string_trim_end(string_trim_start(s)); }

	// Changes the code point at _index_ to a new one. 
	// Since utf-8 code points are not the same byte count,
	// this may need to reorder stuff and expand the string.
	// So we assert that the string is dynamically allocated.
	// You can use the next method in order to get finer control.
	void string_set(string ref s, s64 index, code_point cp);

	void string_insert_at_index(string ref s, s64 index, const char *str, s64 size) {
		index = translate_index(index, string_length(s), true);
		auto *t = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, index);
		array_insert_at_index(s, t - s.Data, str, size);
	}

	void string_insert_at_index(string ref s, s64 index, string str) { string_insert_at_index(s, index, str.Data, str.Count); }

	void string_insert_at_index(string ref s, s64 index, code_point cp) {
		char encodedCp[4];
		utf8_encode_cp(encodedCp, cp);
		string_insert_at_index(s, index, encodedCp, utf8_get_size_of_cp(cp));
	}

	void string_add(string ref s, const char *ptr, s64 size) { array_insert_at_index(s, s.Count, ptr, size); }
	void string_add(string ref s, string b) { array_insert_at_index(s, s.Count, b.Data, b.Count); }
	void string_add(string ref s, code_point cp) { string_insert_at_index(s, string_length(s), cp); }

	auto &operator+=(string ref s, code_point cp) {
		string_add(s, cp);
		return s;
	}

	auto &operator+=(string ref s, const char *str) {
		string_add(s, string(str));
		return s;
	}

	// Remove the first occurrence of a code point.
	// Returns true on success (false if _cp_ was not found in the string).
	bool string_remove(string ref s, code_point cp);

	// Remove code point at specified index.
	void string_remove_at_index(string ref s, s64 index);

	// Remove a range of code points. [begin, end)
	void string_remove_range(string ref s, s64 begin, s64 end);

	void string_remove_all(string ref s, code_point search);
	void string_remove_all(string ref s, string search);
	void string_replace_all(string ref s, code_point search, code_point replace);
	void string_replace_all(string ref s, code_point search, string replace);
	void string_replace_all(string ref s, string search, code_point replace);
	void string_replace_all(string ref s, string search, string replace);

	// Returns a deep copy of _str_ and _count_
	mark_as_leak string make_string(const char *str, s64 count) {
		string result;
		string_add(result, str, count);
		return result;
	}

	// Returns a deep copy of _str_
	mark_as_leak string make_string(const char *str) {
		return make_string(str, c_string_length(str));
	}

	// Returns a deep copy of _src_
	mark_as_leak string clone(string no_copy src) {
		string result;
		string_add(result, src);
		return result;
	}

	// This iterator is to make range based for loops work.
	template <bool Const>
	struct string_iterator {
		using string_t = select_t<Const, const string, string>;

		string_t *String;
		s64 Index;

		string_iterator(string_t *s = null, s64 index = 0) : String(s), Index(index) {}

		string_iterator &operator++() {
			Index += 1;
			return *this;
		}

		string_iterator operator++(s32) {
			string_iterator temp = *this;
			++(*this);
			return temp;
		}

		auto operator<=>(string_iterator other) const { return Index <=> other.Index; };
		auto operator*() { return string_get(*String, Index); }
		operator const char *() const { return utf8_get_pointer_to_cp_at_translated_index(String->Data, String->Count, Index); }
	};

	auto begin(string ref str) { return string_iterator<false>(&str, 0); }
	auto begin(string no_copy str) { return string_iterator<true>(&str, 0); }
	auto end(string ref str) { return string_iterator<false>(&str, string_length(str)); }
	auto end(string no_copy str) { return string_iterator<true>(&str, string_length(str)); }
}

constexpr code_point string_get(string str, s64 index) {
	if (index < 0) {
		// @Speed... should we cache this in _string_?
		// We need to calculate the total length (in code points)
		// in order for the negative index to be converted properly.
		s64 length = string_length(str);
		index = translate_index(index, length);

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

constexpr string string_slice(string str, s64 begin, s64 end) {
	s64 length = string_length(str);
	s64 beginIndex = translate_index(begin, length, true);
	s64 endIndex = translate_index(end, length, true);

	const char *beginPtr = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, beginIndex);
	const char *endPtr = beginPtr;

	// @Speed
	For(range(beginIndex, endIndex)) endPtr += utf8_get_size_of_cp(endPtr);

	return string((char *)beginPtr, (s64)(endPtr - beginPtr));
}

// This cannot be constexpr because predicate cannot be fully constexpr
s64 string_search(string str, delegate<bool(code_point)> predicate, search_options options) {
	if (!str.Data || str.Count == 0) return -1;
	s64 length = string_length(str);
	options.Start = translate_index(options.Start, length, true);
	For(range(options.Start, options.Reversed ? -1 : length, options.Reversed ? -1 : 1)) if (predicate(string_get(str, it))) return it;
	return -1;
}

constexpr s64 string_search(string str, code_point search, search_options options) {
	if (!str.Data || str.Count == 0) return -1;
	s64 length = string_length(str);
	options.Start = translate_index(options.Start, length, true);
	For(range(options.Start, options.Reversed ? -1 : length, options.Reversed ? -1 : 1)) if (string_get(str, it) == search) return it;
	return -1;
}

constexpr s64 string_search(string str, string search, search_options options) {
	if (!str.Data || str.Count == 0) return -1;
	if (!search.Data || search.Count == 0) return -1;

	s64 length = string_length(str);
	options.Start = translate_index(options.Start, length, true);

	s64 searchLength = string_length(search);

	For(range(options.Start, options.Reversed ? -1 : length, options.Reversed ? -1 : 1)) {
		s64 progress = 0;
		for (s64 s = it; progress != searchLength; ++s, ++progress) {
			if (!(string_get(str, s) == string_get(search, progress))) break;
		}
		if (progress == searchLength) return it;
	}
	return -1;
}

constexpr bool string_has(string str, string s) {
	return string_search(str, s) != -1;
}

constexpr bool string_has(string str, code_point cp) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, cp);
	return string_search(str, string(encodedCp, utf8_get_size_of_cp(cp))) != -1;
}

constexpr s64 string_compare(string s, string other) {
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

constexpr s64 string_compare_ignore_case(string s, string other) {
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

constexpr s32 string_compare_lexicographically(string a, string b) {
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

constexpr s32 string_compare_lexicographically_ignore_case(string a, string b) {
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

void string_set(string ref str, s64 index, code_point cp) {
	index = translate_index(index, string_length(str));

	const char *target = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, index);

	char encodedCp[4];
	utf8_encode_cp(encodedCp, cp);

	array_replace_range(str, target - str.Data, target - str.Data + utf8_get_size_of_cp(target), string(encodedCp, utf8_get_size_of_cp(cp)));
}

mark_as_leak char *string_to_c_string(string s, allocator alloc) {
	char *result = malloc<char>({ .Count = s.Count + 1, .Alloc = alloc });
	memcpy(result, s.Data, s.Count);
	result[s.Count] = '\0';
	return result;
}

bool string_remove(string ref s, code_point cp) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, cp);

	s64 index = string_search(s, string(encodedCp, utf8_get_size_of_cp(cp)));
	if (index == -1) return false;

	array_remove_range(s, index, index + utf8_get_size_of_cp(cp));

	return true;
}

void string_remove_at_index(string ref s, s64 index) {
	index = translate_index(index, string_length(s));

	auto *t = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, index);

	s64 b = t - s.Data;
	array_remove_range(s, b, b + utf8_get_size_of_cp(t));
}

void string_remove_range(string ref s, s64 begin, s64 end) {
	s64 length = string_length(s);

	begin = translate_index(begin, length);
	end = translate_index(end, length, true);

	auto *tb = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, begin);
	auto *te = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, end);
	array_remove_range(s, tb - s.Data, te - s.Data);
}

void string_replace_all(string ref s, string search, string replace) {
	array_replace_all(s, search, replace);
}

void string_replace_all(string ref s, code_point search, code_point replace) {
	char encodedOld[4];
	utf8_encode_cp(encodedOld, search);

	char encodedNew[4];
	utf8_encode_cp(encodedNew, replace);

	string_replace_all(s, string(encodedOld, utf8_get_size_of_cp(encodedOld)), string(encodedNew, utf8_get_size_of_cp(encodedNew)));
}

void string_remove_all(string ref s, code_point search) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, search);

	string_replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), string(""));
}

void string_remove_all(string ref s, string search) { string_replace_all(s, search, string("")); }

void string_replace_all(string ref s, code_point search, string replace) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, search);

	string_replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), replace);
}

void string_replace_all(string ref s, string search, code_point replace) {
	char encodedCp[4];
	utf8_encode_cp(encodedCp, replace);

	string_replace_all(s, search, string(encodedCp, utf8_get_size_of_cp(encodedCp)));
}

LSTD_END_NAMESPACE
