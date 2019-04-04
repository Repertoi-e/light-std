#pragma once

/// A header which provides type definitions as well as other helper macros

#include "types.h"

// Convenience storage literal operators, allows for specifying sizes like this:
//  size_t a = 10_MiB;

// _B For completeness
constexpr size_t operator"" _B(u64 i) { return (size_t)(i); }
constexpr size_t operator"" _KiB(u64 i) { return (size_t)(i) << 10; }
constexpr size_t operator"" _MiB(u64 i) { return (size_t)(i) << 20; }
constexpr size_t operator"" _GiB(u64 i) { return (size_t)(i) << 30; }

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

// Go-style defer
//
//  defer {
//      ...; // Gets called on scope exit
//  };
//
#ifndef defer
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

#define DEFER_INTERNAL_(LINE) LSTD_defer##LINE
#define DEFER_INTERNAL(LINE) DEFER_INTERNAL_(LINE)
#define defer auto DEFER_INTERNAL(__LINE__) = LSTD_NAMESPACE ::Defer_Dummy{} *[&]()
#endif

// Shortcut macros for "for each" loops (really up to personal style if you want to use this)
//
//  For(array) print(it);
//
#define For_as(x, in) for (const auto &x : in)
#define For(in) For_as(it, in)

LSTD_BEGIN_NAMESPACE

// Base classes to reduce boiler plate code
struct non_copyable {
   private:
    non_copyable() = default;
    ~non_copyable() = default;

    non_copyable(const non_copyable &) = delete;
    non_copyable &operator=(const non_copyable &) = delete;
};

struct non_movable {
   private:
    non_movable() = default;
    ~non_movable() = default;

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
   private:
    struct iterator {
        s64 I, Step;

        constexpr iterator(s64 i, s64 step = 1) : I(i), Step(step) {}

        operator s32() const { return (s32) I; }
        operator s64() const { return I; }

        constexpr s64 operator*() const { return (s32) I; }
        constexpr const iterator &operator++() { return I += Step, *this; }

        constexpr iterator operator++(int) {
            iterator temp(*this);
            return I += Step, temp;
        }

        constexpr bool operator==(const iterator &other) const { return Step < 0 ? (I <= other.I) : (I >= other.I); }
        constexpr bool operator!=(const iterator &other) const { return Step < 0 ? (I > other.I) : (I < other.I); }
    };

    iterator _Begin, _End;

   public:
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

LSTD_END_NAMESPACE
