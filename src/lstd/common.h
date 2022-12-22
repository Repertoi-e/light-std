#pragma once

//
// A header which imports common types, numeric info,
//     common math functions, definitions for the macros: 
//	assert, defer, For, For_enumerate ...
//		static_for, range 
// ... and others.
// 
// And also memory stuff:
//     memcpy, memset, memset0, memcmp 
//
// Really very common lightweight stuff that's used all the time.
//

import lstd.math; // also imports lstd.type_info, lstd.numeric, and lstd.ieee

//
// Cephes provides our replacement for the math functions found in virtually all standard libraries.
// Also provides functions for extended precision arithmetic, statistical functions, physics, astronomy, etc.
// https://www.netlib.org/cephes/
// Note: We don't include everything from it, just cmath for now.
//       Statistics is a thing we will most definitely include as well in the future.
//       Everything else you can include on your own in your project (we don't want to be bloat-y).
//
// Note: Important difference,
// atan2's return range is 0 to 2PI, and not -PI to PI (as per normal in the C standard library).
//
// Parts of the source code that we modified are marked with :WEMODIFIEDCEPHES:
//

/*
Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1995, 2000 by Stephen L. Moshier
*/
#include "third_party/cephes/maths_cephes.h"

#include "common/namespace.h"
#include "common/platform.h"
#include "common/debug_break.h"
#include "common/defer.h"
#include "common/assert.h"
#include "common/for.h"
#include "common/enumerate.h"
#include "common/fmt.h"
#include "common/namespace.h"

#include "context/context.h"
#include "memory/allocation.h"

// :AvoidSTL:
// Usually we build without dependencies on the STL,
// but if LSTD_DONT_DEFINE_STD is defined, we include
// the initializer list and space ships defined in the STL
// to avoid conflicts with our implementations.
#if defined LSTD_DONT_DEFINE_STD
#include <stdarg.h>
#include <initializer_list>
#include <compare>
#else
#include "cpp_compatibility/arg.h"
import lstd.initializer_list_replacement;
import lstd.space_ship_replacement;
#endif

import lstd.source_location;

//
// Some personal preferences:
// 
// I prefer to type null over nullptr but they are exactly the same
using null_t = decltype(nullptr);
inline const null_t null = nullptr;

import lstd.range;

// Semantics to avoid the use of & when the symbol is not used as a unary or binary operator.
//
// e.g.
//      void print_array_to_file(array<u8> no_copy bytes) { ... }
//      void modify_array(array<u8> ref bytes) { ... }
//
#define no_copy const &
#define ref &

// Used to mark functions for which the caller is supposed to free the result.
// This at leasts makes the compiler warn the caller if they've decided to discard the result.
//
// e.g.
//		mark_as_leak string make_string(...) { ... }
//
#define mark_as_leak [[nodiscard("Leak")]]

//
// Personal preference, use 
//
#define cast(x) (x)

// Helper macro for, e.g flag enums
//
// enum flags {
//	Flag_1 = BIT(0),
//  Flag_1 = BIT(1)
//  Flag_1 = BIT(2)
//  ...
// };
//
#define BIT(x) (1 << (x))

// Gives the offset of a member in a struct (in bytes)
#define offset_of(s, field) ((u64) & ((s *) (0))->field)

// Tau supremacy https://tauday.com/tau-manifesto
#define TAU 6.283185307179586476925286766559
#define PI (TAU / 2)

//
// Convenience storage literal operators, allows for specifying sizes like this:
//  s64 a = 10_MiB;
//  s64 a = 20_billion;
//
// The International Electronic Commission established the term kibibyte 
// for 1024 bytes, because the metric system already has a use for the 
// prefix "kilo" meaning a thousand. So 1 KB = 1000 bytes.
// 
// In practice, however, when buying storage or downloading files or
// looking in Windows explorer, KB has the meaning of 1024 bytes.
// Suddenly switching to KiB for no particular reason (except having
// the feeling of being correct or superior that you know a term
// which others don't) would cause confusion to users.
// 
// However, we are programmers. And in general the decision of the IEC
// does sound logical. So for the sake of being exact we will name
// these literals with the proper term.
//
// 
// _B is for completeness, really useless though 
constexpr u64 operator"" _B(u64 i) { return i; }
constexpr u64 operator"" _KiB(u64 i) { return i << 10; }
constexpr u64 operator"" _MiB(u64 i) { return i << 20; }
constexpr u64 operator"" _GiB(u64 i) { return i << 30; }

constexpr u64 operator"" _thousand(u64 i) { return i * 1000; }
constexpr u64 operator"" _million(u64 i) { return i * 1000000; }
constexpr u64 operator"" _billion(u64 i) { return i * 1000000000; }

LSTD_BEGIN_NAMESPACE

template <typename T>
inline void swap(T &a, T &b) {
	T c = a;
	a = b;
	b = c;
}

template <typename T, s64 N>
inline void swap(T(&a)[N], T(&b)[N]) {
	For(range(N)) swap(a[it], b[it]);
}

// @Volatile: README.md
// :TypePolicy:
//
// Ideal: keep it simple.
//
// - Keep it data oriented.
//   Programs work with data. Design your data so it makes the solution straightforward and minimize abstraction layers.
// - Use struct instead of class, keep everything public.
// - Always provide a default constructor (implicit or by "T() {}")
// - copy/move constructors and destructors are banned. No excuse.
// - No throwing of exceptions, .. ever, .. anywhere. No excuse.
//   They make your code complicated. When you can't handle an error and need to exit from a function, return multiple values.
//   C++ doesn't really help with this, but you can use C++17 structured bindings, e.g.:
//          auto [content, success] = path_read_entire_file("data/hello.txt");
//
//
// Some examples:
//
// _array_ is this library implemented the following way:
//     _array_ is a struct that contains 2 fields (Data and Count).
//
//     It has no sense of ownership. That is determined explictly in code and by the programmer.
//
//     By default arrays are views, to make them dynamic, call     reserve(arr, ...).
//     After that you can modify them (add/remove elements etc.)
//
//     You can safely pass around copies and return arrays from functions because
//     there is no hidden destructor which will free the memory.
//
//     When a dynamic array is no longer needed call    free(arr);
//
//
//     We provide a defer macro which runs at the end of the scope (like a destructor),
//     you can use this for functions which return from multiple places,
//     so you are absolutely sure    free(arr.Data) is ran and there were no leaks.
//
//
//     All of this allows to skip writing copy/move constructors/assignment operators.
//
// _string_s are like arrays, but different types to avoid conflicts with indices. 
// Indices are to utf-8 code points instead of to bytes.
// They are not null-terminated, which means that taking substrings doesn't allocate memory.
// But all of the above (for arrays) applies to them as well.
//
//
//     // Constructed from a zero-terminated string buffer. Doesn't allocate memory.
//     // Like arrays, strings are views by default.
//     string path = "./data/";
//	   reserve(path);
//     defer(free(path));
//     path += "output.txt";
// 
// or:
// 
//	   string path = make_string("./data/");
//     defer(free(path));
//     path += "output.txt";
//
//     string pathWithoutDot = string_slice(path, 2, -1);
//
// To make a deep copy of an array use clone().
// e.g.         string newPath = clone(path); // Allocates a new buffer and copies contents in _path_
//

template <typename T>
inline T *memmove(T *dst, const T *src, s64 numInBytes) {
	For(range(numInBytes / sizeof(T) - 1, -1, -1)) dst[it] = src[it];
	return dst;
}

template <typename T>
inline T *memcpy(T *dst, const T *src, s64 numInBytes) {
	if (dst > src && (s64)(dst - src) < (numInBytes / (s64)sizeof(T))) {
		//
		// Careful. Buffers overlap. You should use memmove in this case.
		// 
		// If this bug isn't caught until Release, then bad shit happens.
		// So in order to make it work nevertheless we do memmove.
		// I wish the C standard didn't make a distinction between the
		// two functions, but we're stuck with that.
		// 
		// This makes calling memmove superfluous, and personally, 
		// I'm ok with that.
		return memmove(dst, src, numInBytes);
	}
	else {
		For(range(numInBytes / sizeof(T))) dst[it] = src[it];
	}
	return dst;
}

template <typename T>
inline T *memset(T *dst, T value, u64 numInBytes) {
	For(range(numInBytes / sizeof(T))) dst[it] = value;
	return dst;
}

// Non-standard, but useful.
template <typename T>
inline T *memset0(T *dst, u64 numInBytes) {
	return memset(dst, T(0), numInBytes);
}

template <typename T>
inline s32 memcmp(const T *s1, const T *s2, s64 numInBytes) {
	For(range(numInBytes / sizeof(T))) {
		if (!(*s1 == *s2)) return *s1 - *s2;
		++s1, ++s2;
	}
	return 0;
}

LSTD_END_NAMESPACE
