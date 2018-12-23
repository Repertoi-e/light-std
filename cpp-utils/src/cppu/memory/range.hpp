#pragma once

#include "../context.hpp"

CPPU_BEGIN_NAMESPACE

struct Range {
   private:
    struct Iterator {
        s64 I;
        s64 Step;

        constexpr Iterator(s64 i, s64 step = 1) : I(i), Step(step) {}

        operator s32 const &() { return (s32) I; }
        operator s64 const &() { return I; }

        constexpr s32 operator*() const { return (s32) I; }

        constexpr const Iterator &operator++() {
            I += Step;
            return *this;
        }

        constexpr Iterator operator++(int) {
            Iterator temp(*this);
            I += Step;
            return temp;
        }

        constexpr b32 operator==(const Iterator &other) const { return Step < 0 ? (I <= other.I) : (I >= other.I); }
        constexpr b32 operator!=(const Iterator &other) const { return Step < 0 ? (I > other.I) : (I < other.I); }
    };

    Iterator Begin;
    Iterator End;

   public:
    constexpr Range(u64 stop) : Begin(0), End(stop) {}
    constexpr Range(s64 start, s64 stop, s64 step) : Begin(start, step), End(stop) {}

    constexpr Iterator begin() const { return Begin; }
    constexpr Iterator end() const { return End; }
};

constexpr Range range(u64 stop) { return Range(stop); }
constexpr Range range(s64 start, s64 stop) { return Range(start, stop, 1); }
constexpr Range range(s64 start, s64 stop, s64 step) { return Range(start, stop, step); }

CPPU_END_NAMESPACE
