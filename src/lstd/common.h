#pragma once

//
// A header which defines common types, numeric info,
//     bits operations, atomic operations, common math functions,
//     definitions for the macros: assert, defer, For, For_enumerate ...
//     static_for, range 
// ... and other utils.
// 
// And also memory stuff:
//     memcpy, memset, memset0, memcmp 
//
// Really any stuff that doesn't deserve to be it's own module.
//

#include "common/types_and_range.h"
#include "common/atomic.h"
#include "common/bits.h"
#include "common/context.h"
#include "common/debug_break.h"
#include "common/defer_assert_for_and_utils.h"
#include "common/fmt.h"
#include "common/math.h"
#include "common/memory.h"
#include "common/namespace.h"
#include "common/numeric_info.h"
#include "common/u128.h"

// Tau supremacy
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

//
// Loop that gets unrolled at compile-time, this avoids copy-pasta
// or using macros in order to be sure the code gets unrolled properly.
//
template <s64 First, s64 Last, typename Lambda>
void static_for(Lambda ref f) {
	if constexpr (First < Last) {
		f(types::integral_constant<s64, First>{});
		static_for<First + 1, Last>(f);
	}
}

template <typename T>
constexpr void swap(T &a, T &b) {
	T c = a;
	a = b;
	b = c;
}

template <typename T, s64 N>
constexpr void swap(T(&a)[N], T(&b)[N]) {
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
//     By default arrays are views, to make them dynamic, call     make_dynamic(&arr).
//     After that you can modify them (add/remove elements etc.)
//
//     You can safely pass around copies and return arrays from functions because
//     there is no hidden destructor which will free the memory.
//
//     When a dynamic array is no longer needed call    free(arr.Data);
//
//
//     We provide a defer macro which runs at the end of the scope (like a destructor),
//     you can use this for functions which return from multiple places,
//     so you are absolutely sure    free(arr.Data) is ran and there were no leaks.
//
//
//     All of this allows to skip writing copy/move constructors/assignment operators.
//
// _string_s are just array<char>. All of this applies to them as well.
// They are not null-terminated, which means that taking substrings doesn't allocate memory.
//
//
//     // Constructed from a zero-terminated string buffer. Doesn't allocate memory.
//     // Like arrays, strings are views by default.
//     string path = "./data/";
//     make_dynamic(&path);         // Allocates a buffer and copies the string it was pointing to
//     defer(free(path.Data));
//
//     append(&path, "output.txt");
//
//     string pathWithoutDot = substring(path, 2, -1);
//
// To make a deep copy of an array use clone().
// e.g.         string newPath = clone(path); // Allocates a new buffer and copies contents in _path_
//

//
// We used to provide separate constexpr and optimized runtime 
// implementations of copy_memory, fill_memory, compare_memory
// that use SIMD and AVX instructions. However, they caused
// quite a bit of hard to track down bugs, because they weren't 
// 100% reliable. Moreover, if memcpy is the bottle neck of your
// program's performance, perhaps you should write your own
// specialized implementation that has some extra assumptions baked
// in (which only you can know!), instead of relying on our previous 
// "general fast" implementation that also had a lot of branches.
// 
// @Speed Something can be said about having routines that work with 
// multiple bytes at once (e.g. copying/filling/comparing words to words), 
// but they don't work with constexpr, which doesn't support type pruning. 
//
// Constexpr doesn't work with void * as well, in general, void * 
// specialization functions will be on par or faster than the templated 
// ones for custom types, because C++ likes to be annoying as fuck 
// and generates default copy constructors which do member-wise
// copy (even for trivially-copyable types).
// 
// So I recommend always casting values to (char *) as normal.
//

template <typename T>
constexpr T *memmove(T *dst, const T *src, s64 numInBytes) {
	For(range(numInBytes / sizeof(T) - 1, -1, -1)) dst[it] = src[it];
	return dst;
}

template <typename T>
constexpr T *memcpy(T *dst, const T *src, s64 numInBytes) {
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
constexpr T *memset(T *dst, T value, u64 numInBytes) {
	For(range(numInBytes / sizeof(T))) dst[it] = value;
	return dst;
}

// Non-standard, but useful.
template <typename T>
constexpr T *memset0(T *dst, u64 numInBytes) {
	return memset(dst, T(0), numInBytes);
}

template <typename T>
constexpr s32 memcmp(const T *s1, const T *s2, s64 numInBytes) {
	For(range(numInBytes / sizeof(T))) {
		if (!(*s1 == *s2)) return *s1 - *s2;
		++s1, ++s2;
	}
	return 0;
}

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

LSTD_END_NAMESPACE
