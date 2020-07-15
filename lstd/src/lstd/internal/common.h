#pragma once

/// A header which provides type definitions as well as other helper macros

#include "../types.h"
#include "debug_break.h"

// Convenience storage literal operators, allows for specifying sizes like this:
//  s64 a = 10_MiB;

// _B For completeness
constexpr s64 operator"" _B(u64 i) { return (s64)(i); }
constexpr s64 operator"" _KiB(u64 i) { return (s64)(i) << 10; }
constexpr s64 operator"" _MiB(u64 i) { return (s64)(i) << 20; }
constexpr s64 operator"" _GiB(u64 i) { return (s64)(i) << 30; }

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

// Used in macros to get "unique" variable names
#define LINE_NAME(name) _MACRO_CONCAT(name, __LINE__)
#define _MACRO_DO_CONCAT(s1, s2) s1##s2
#define _MACRO_CONCAT(s1, s2) _MACRO_DO_CONCAT(s1, s2)

// Go-style defer
//
//  defer(...);
//  defer({
//      ...;
//  });
//
// The statements inside get called on scope exit
//
#undef defer

LSTD_BEGIN_NAMESPACE
struct Defer_Dummy {};
template <typename F>
struct Deferrer {
    F Func;
    ~Deferrer() { Func(); }
};
template <typename F>
Deferrer<F> operator*(Defer_Dummy, F func) {
    return {func};
}
LSTD_END_NAMESPACE

#define defer(x) auto LINE_NAME(LSTD_defer) = LSTD_NAMESPACE ::Defer_Dummy{} * [&]() { x; }

#undef assert

#if defined DEBUG || defined RELEASE
#define assert(condition) (!!(condition)) ? (void) 0 : debug_break()
#else
#define assert(condition) ((void) 0)
#endif

// Shortcut macros for "for each" loops (really up to personal style if you want to use this)
//
//  For(array) print(it);
//
#define For_as(x, in) for (auto &&x : in)
#define For(in) For_as(it, in)

LSTD_BEGIN_NAMESPACE

// Base classes to reduce boiler plate code
struct non_copyable {
   protected:
    non_copyable() = default;
    ~non_copyable() = default;

   private:
    non_copyable(const non_copyable &) = delete;
    non_copyable &operator=(const non_copyable &) = delete;
};

struct non_movable {
   protected:
    non_movable() = default;
    ~non_movable() = default;

   private:
    non_movable(non_movable &&) = delete;
    non_movable &operator=(non_movable &&) = delete;
};

struct non_assignable {
   private:
    non_assignable &operator=(const non_assignable &) = delete;
    non_assignable &operator=(non_assignable &&) = delete;
};

// Python-like range functionality
// e.g.
//
//  for (auto it : range(20))        // [0, 20)
//  for (auto it : range(3, 10, 2))  // every second integer (step 2) in [3, 10)
//  for (auto it : range(10, 0, -1)) // reverse [10, 0)
//
struct range {
    struct iterator {
        s64 I;
        s64 Step;

        constexpr iterator(s64 i, s64 step = 1) : I(i), Step(step) {}

        operator s32() const { return (s32) I; }
        operator s64() const { return I; }

        constexpr s64 operator*() const { return (s32) I; }
        constexpr iterator operator++() { return I += Step, *this; }

        constexpr iterator operator++(int) {
            iterator temp(*this);
            return I += Step, temp;
        }

        constexpr bool operator==(iterator other) const { return Step < 0 ? (I <= other.I) : (I >= other.I); }
        constexpr bool operator!=(iterator other) const { return Step < 0 ? (I > other.I) : (I < other.I); }
    };

    iterator _Begin;
    iterator _End;

    constexpr range(s64 start, s64 stop, s64 step) : _Begin(start, step), _End(stop) {}
    constexpr range(s64 start, s64 stop) : range(start, stop, 1) {}
    constexpr range(u64 stop) : range(0, stop, 1) {}

    // Checks if a value is inside the given range.
    // This also accounts for stepping.
    constexpr bool has(s64 value) const {
        if (_Begin.Step > 0 ? (value >= _Begin.I && value < _End.I) : (value > _End.I && value <= _Begin.I)) {
            s64 diff = value - _Begin.I;
            if (diff % _Begin.Step == 0) {
                return true;
            }
        }
        return false;
    }

    constexpr iterator begin() const { return _Begin; }
    constexpr iterator end() const { return _End; }
};

// @TODO This needs updating...
//
// @Volatile: README.md
// Type policy:
//
// Aim of this policy:
// - Dramatically reduce complexity and code size (both library AND user side!) UNLESS that comes at a run-time cost
//
// - Always provide a default constructor (implicit or by "T() = default")
// - Every data member should have the same access control (everything should be public or private or protected)
// - Strive to make classes/structures/objects (whatever you wanna call them) data oriented.
//   Programs work with data. Design your data so it makes the solution straightforward and minimize abstraction layers.
// - No user defined copy/move constructors
// - No throwing of exceptions, anywhere
//
// "No user defined copy/move constructors":
//   This may sound crazy if you have a type that owns memory (how would you deep copy the contents and not
//   just the pointer when you copy the object?).
//   All allocations in this library contain a header with some information about the allocation.
//   One of the pieces of information is a pointer to the object that owns the memory.
//   That pointer is set manually using functions from "memory/owner_pointers.h".
//   So _string_ is implemented the following way:
//     _string_ is a struct that contains a pointer to a byte buffer and 2 fields containing pre-calculated
//     utf8 code unit and code point lengths, as well as a field _Reserved_ that contains the number of
//     bytes allocated by that string (default is 0).
//
//     A string may own its allocated memory or it may not, which is determined by encoding the _this_ pointer
//     in the allocation header when the string reserves a buffer.
//     That way when you shallow copy the string, the _this_ pointer is obviously different (because it is a
//     different object) and when the copied string gets destructed it doesn't free the memory (it doesn't own it).
//     Only when the original string gets destructed does the memory get freed and any shallow copies of it
//     are invalidated (they point to freed memory).
//
//     When a string gets contructed from a literal it doesn't allocate memory.
//     _Reserved_ is 0 and the object works like a view.
//
//     If the string was constructed from a literal, shallow copy of another string or a byte buffer,
//     when you call modifying methods (like appending, inserting code points, etc.) the string allocates a buffer,
//     copies the old one and now owns memory.
//
//
//     _clone(T *dest, T src)_ is a global function that ensures a deep copy of the argument passed.
//     Objects that own memory (like string) overload clone() and make sure the copy reserves a buffer and copies
//     the data to it.
//
//     _move(T *dest, T *src)_ is global function that transfers ownership.
//     The buffer in _src_ (iff _src_ owns it) is now owned by _dest_ (_src_ becomes simply a view into _dest_).
//     So _move_ is cheaper than _clone_ and is used for example when inserting objects into an array.
//
//     ! Note: _clone_ and _move_ work on all types (unless overloaded they do a shallow copy).
//     They are the recommended way to implement functionality normally done in copy/move c-tors.
//
// "No throwing of exceptions, anywhere"
//   Exceptions make your code complicated. They are a good way to handle errors in small examples,
//   but don't really help in large programs/projects. You can't be 100% sure what can throw where and when
//   thus you don't really know what your program is doing (you aren't sure it even works 100% of the time).
//   You should design code in such a way that errors can't occur (or if they do - handle them, not just bail,
//   and when even that is not possible - stop execution).
//
// Every type in this library complies with this policy

// Global function that is supposed to ensure a deep copy of the argument passed
// By default, a shallow copy is done (to make sure it can be called on all types)
template <typename T>
T *clone(T *dest, T src) {
    *dest = src;
    return dest;
}

// Global function that is supposed to ensure transfer of ownership without the overhead of cloning
// By default, a normal copy is done (to make sure it can be called on all types)
// Returns _dest_
template <typename T>
constexpr T *move(T *dest, T *src) {
    *dest = *src;
    return dest;
}

template <typename T>
constexpr void swap(T &a, T &b) {
    T c;
    move(&c, &a);
    move(&a, &b);
    move(&b, &c);
}

template <typename T, s64 N>
constexpr void swap(T (&a)[N], T (&b)[N]) {
    For(range(N)) swap(a[it], b[it]);
}

//
// copy_memory, fill_memory, compare_memory and SSE optimized implementations when on x86 architecture
// (implemenations in memory/memory.cpp)
//

// In this library, copy_memory works like memmove in the std (handles overlapping buffers)
// :CopyMemory (declared in types.h also to avoid circular includes)
extern void (*copy_memory)(void *dest, const void *src, s64 num);
constexpr void const_copy_memory(void *dest, const void *src, s64 num) {
    auto *d = (char *) dest;
    auto *s = (const char *) src;

    if (d <= s || d >= (s + num)) {
        // Non-overlapping
        while (num--) {
            *d++ = *s++;
        }
    } else {
        // Overlapping
        d += num - 1;
        s += num - 1;

        while (num--) {
            *d-- = *s--;
        }
    }
}

extern void (*fill_memory)(void *dest, char value, s64 num);
constexpr void const_fill_memory(void *dest, char value, s64 num) {
    auto d = (char *) dest;
    while (num-- > 0) *d++ = value;
}

inline void zero_memory(void *dest, s64 num) { return fill_memory(dest, 0, num); }
constexpr void const_zero_memory(void *dest, s64 num) { return const_fill_memory(dest, 0, num); }

// compare_memory returns the index of the first byte that is different
// e.g: calling with
//		*ptr1 = 00000011
//		*ptr1 = 00100001
//	returns 2
// If the memory regions are equal, the returned value is -1
extern s64 (*compare_memory)(const void *ptr1, const void *ptr2, s64 num);
constexpr s64 const_compare_memory(const void *ptr1, const void *ptr2, s64 num) {
    auto *s1 = (const char *) ptr1;
    auto *s2 = (const char *) ptr2;

    For(range(num)) if (*s1++ != *s2++) return it;
    return -1;
}

#define POWERS_OF_10(factor)                                                                                        \
    factor * 10, factor * 100, factor * 1000, factor * 10000, factor * 100000, factor * 1000000, factor * 10000000, \
        factor * 100000000, factor * 1000000000

constexpr u32 POWERS_OF_10_32[] = {1, POWERS_OF_10(1)};
constexpr u64 POWERS_OF_10_64[] = {1, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};

constexpr u32 ZERO_OR_POWERS_OF_10_32[] = {0, POWERS_OF_10(1)};
constexpr u64 ZERO_OR_POWERS_OF_10_64[] = {0, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};
#undef POWERS_OF_10

union ieee754_f32 {
    f32 F;
    u32 W;

    // This is the IEEE 754 single-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 8;
        u32 M : 23;
#else
        u32 M : 23;
        u32 E : 8;
        u32 S : 1;
#endif
    } ieee;

    // This format makes it easier to see if a NaN is a signalling NaN.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 8;
        u32 N : 1;
        u32 M : 22;
#else
        u32 M : 22;
        u32 N : 1;
        u32 E : 8;
        u32 S : 1;
#endif
    } ieee_nan;
};

union ieee754_f64 {
    f64 F;
    u64 W;

    struct {
#if ENDIAN == BIG_ENDIAN
        u32 MSW;
        u32 LSW;
#else
        u32 LSW;
        u32 MSW;
#endif
    };

    // This is the IEEE 754 single-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 11;
        u32 M0 : 20;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 20;
        u32 E : 11;
        u32 S : 1;
#endif
    } ieee;

    // This format makes it easier to see if a NaN is a signalling NaN.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 11;
        u32 N : 1;
        u32 M0 : 19;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 19;
        u32 N : 1;
        u32 E : 11;
        u32 S : 1;
#endif
    } ieee_nan;
};

// @Wrong
// This seems wrong, not sure.
// sizeof(ieee854_lf64) is 16 but sizeof(long double) in MSVC is 8
union ieee854_lf64 {
    lf64 F;
    u64 W;

    struct {
#if ENDIAN == BIG_ENDIAN
        u32 MSW;
        u32 LSW;
#else
        u32 LSW;
        u32 MSW;
#endif
    };

    // This is the IEEE 854 double-extended-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 15;
        u32 Empty : 16;
        u32 M0 : 32;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 32;
        u32 E : 15;
        u32 S : 1;
        u32 Empty : 16;
#endif
    } ieee;

    // This is for NaNs in the IEEE 854 double-extended-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 15;
        u32 Empty : 16;
        u32 One : 1;
        u32 N : 1;
        u32 M0 : 30;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 30;
        u32 N : 1;
        u32 One : 1;
        u32 E : 15;
        u32 S : 1;
        u32 Empty : 16;
#endif
    } ieee_nan;
};

#if COMPILER == MSVC
#pragma intrinsic(_BitScanReverse)
inline u32 msb(u32 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse(&r, x);
    return 31 - r;
}

#pragma intrinsic(_BitScanReverse64)
inline u32 msb_64(u64 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse64(&r, x);
    return 63 - r;
}

#pragma intrinsic(_BitScanForward)
inline u32 lsb(u32 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanForward(&r, x);
    return r;
}

#pragma intrinsic(_BitScanForward64)
inline u32 lsb_64(u64 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanForward64(&r, x);
    return r;
}
#else
#define msb(n) __builtin_clz(n)
#define msb_64(n) __builtin_clzll(n)
#define lsb(n) __builtin_ctz(n)
#define lsb_64(n) __builtin_ctzll(n)
#endif

constexpr u32 rotate_left_32(u32 x, u32 bits) { return (x << bits) | (x >> (32 - bits)); }
constexpr u64 rotate_left_64(u64 x, u32 bits) { return (x << bits) | (x >> (64 - bits)); }

constexpr u32 rotate_right_32(u32 x, u32 bits) { return (x >> bits) | (x << (32 - bits)); }
constexpr u64 rotate_right_64(u64 x, u32 bits) { return (x >> bits) | (x << (64 - bits)); }

// Useful: http://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign

#define u32_has_zero(v) (((v) -0x01010101UL) & ~(v) &0x80808080UL)
#define u32_has_value(x, n) (u32_has_zero((x) ^ (~0UL / 255 * (u8)(n))))

#define u32_has_less(x, n) (((x) - ~0UL / 255 * (u8)(n)) & ~(x) & ~0UL / 255 * 128)
#define u32_count_less(x, n) \
    (((~0UL / 255 * (127 + (n)) - ((x) & ~0UL / 255 * 127)) & ~(x) & ~0UL / 255 * 128) / 128 % 255)

#define u32_has_more(x, n) (((x) + ~0UL / 255 * (127 - (u8)(n)) | (x)) & ~0UL / 255 * 128)
#define u32_count_more(x, n) \
    (((((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(n)) | (x)) & ~0UL / 255 * 128) / 128 % 255)

#define u32_likely_has_between(x, m, n) \
    ((((x) - ~0UL / 255 * (u8)(n)) & ~(x) & ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(m))) & ~0UL / 255 * 128)
#define u32_has_between(x, m, n)                                       \
    ((~0UL / 255 * (127 + (u8)(n)) - ((x) & ~0UL / 255 * 127) & ~(x) & \
      ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(m))) &       \
     ~0UL / 255 * 128)
#define u32_count_between(x, m, n) (u32_has_between(x, m, n) / 128 % 255)

#if COMPILER == MSVC
#pragma warning(push)
#pragma warning(disable : 4146)
#endif

template <typename T>
constexpr enable_if_t<is_integer_v<T>> set_bit(T *number, T bit, bool value) {
    auto enabled = (make_unsigned_t<T>) value;
    *number ^= (-enabled ^ *number) & bit;
}

#if COMPILER == MSVC
#pragma warning(pop)
#endif

#define INTEGRAL_FUNCTION_CONSTEXPR(return_type) \
    template <typename T>                        \
    constexpr enable_if_t<is_integral_v<T>, return_type>

INTEGRAL_FUNCTION_CONSTEXPR(bool)
is_pow_of_2(T number) { return (number & (number - 1)) == 0; }

INTEGRAL_FUNCTION_CONSTEXPR(T)
const_abs(T number) {
    auto s = number >> (sizeof(T) * 8 - 1);
    return (number ^ s) - s;
}

constexpr f32 const_abs(f32 number) {
    ieee754_f32 format = {number};
    format.ieee.S = 0;
    return format.F;
}

constexpr f64 const_abs(f64 number) {
    ieee754_f64 format = {number};
    format.ieee.S = 0;
    return format.F;
}

template <typename T>
constexpr enable_if_t<is_integral_v<T> && numeric_info<T>::is_signed, bool> sign_bit(T number) {
    return number < 0;
}

template <typename T>
constexpr enable_if_t<is_integral_v<T> && !numeric_info<T>::is_signed, bool> sign_bit(T) {
    return false;
}

constexpr bool sign_bit(f32 number) {
    ieee754_f32 format = {number};
    return format.ieee.S;
}

constexpr bool sign_bit(f64 number) {
    ieee754_f64 format = {number};
    return format.ieee.S;
}

// Handles zero as well
template <typename T>
constexpr s32 sign(T number) {
    if (number == T(0)) return 0;
    return sign_bit(number) ? -1 : 1;
}

template <typename T>
constexpr s32 sign_no_zero(T number) {
    return sign_bit(number) ? -1 : 1;
}

constexpr bool is_inf(f32 number) {
    ieee754_f32 format = {number};
    return format.ieee.E == 0xff && format.ieee.M == 0;
}

constexpr bool is_inf(f64 number) {
    ieee754_f64 format = {number};
#if ENDIAN == BIG_ENDIAN
    // @Wrong
    // Haven't tested this
    return ((u32) format.W & 0xffffff7f) == 0x0000f07f && ((u32)(format.W >> 32) == 0);
#else
    return ((u32)(format.W >> 32) & 0x7fffffff) == 0x7ff00000 && ((u32) format.W == 0);
#endif
}

constexpr bool is_nan(f32 number) {
    ieee754_f32 format = {number};
    return format.ieee.E == 0xff && format.ieee.M != 0;
}

constexpr bool is_nan(f64 number) {
    ieee754_f64 format = {number};
#if ENDIAN == BIG_ENDIAN
    // @Wrong
    // Haven't tested this
    return ((u32) format.W & 0xffffff7f) + ((u32)(format.W >> 32) != 0) > 0x0000f07f;
#else
    return ((u32)(format.W >> 32) & 0x7fffffff) + ((u32) format.W != 0) > 0x7ff00000;
#endif
}

template <typename T>
constexpr T const_min(T x, T y) {
    return x < y ? x : y;
}

template <typename T>
constexpr T const_max(T x, T y) {
    return x > y ? x : y;
}

template <typename T>
constexpr T min(T x, T y) {
    return x < y ? x : y;
}

template <typename T>
constexpr T max(T x, T y) {
    return x > y ? x : y;
}

// Defined in common.cpp
f32 min(f32 x, f32 y);
f32 max(f32 x, f32 y);
f64 min(f64 x, f64 y);
f64 max(f64 x, f64 y);

template <typename T>
enable_if_t<is_integral_v<T>, T> ceil_pow_of_2(T v) {
    v--;
    for (s64 i = 1; i < sizeof(T) * 8; i *= 2) v |= v >> i;
    return ++v;
}

#undef INTEGRAL_FUNCTION_CONSTEXPR

// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline u32 count_digits(u64 n) {
    s32 t = (64 - msb_64(n | 1)) * 1233 >> 12;
    return (u32) t - (n < ZERO_OR_POWERS_OF_10_64[t]) + 1;
}

template <u32 Bits, typename T>
constexpr u32 count_digits(T value) {
    T n = value;
    u32 numDigits = 0;
    do {
        ++numDigits;
    } while ((n >>= Bits) != 0);
    return numDigits;
}

// All of these return the value after the operation
#if COMPILER == MSVC
#define atomic_inc(ptr) _InterlockedIncrement((ptr))
#define atomic_inc_64(ptr) _InterlockedIncrement64((ptr))
#define atomic_add(ptr, value) _InterlockedAdd((ptr), value)
#define atomic_add_64(ptr, value) _InterlockedAdd64((ptr), value)
#else
#define atomic_inc(ptr) __sync_add_and_fetch((ptr), 1)
#define atomic_inc_64(ptr) __sync_add_and_fetch((ptr), 1)
#error atomic_add and atomic_add_64
#endif

template <typename T>
constexpr T const_exp10(s32 exponent) {
    return exponent == 0 ? T(1) : T(10) * const_exp10<T>(exponent - 1);
}

template <typename T>
struct delegate;

// Schedule a function to run when this library uninitializes - before any global C++ destructors are called
// and before the CRT (if we are linking against it) unitializes.
void run_at_exit(const delegate<void()> &function);

LSTD_END_NAMESPACE

#if COMPILER != MSVC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
