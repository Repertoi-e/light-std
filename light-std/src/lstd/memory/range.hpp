#pragma once

#include "../common.hpp"

LSTD_BEGIN_NAMESPACE

struct range {
   private:
    struct Iterator {
        s64 I;
        s64 Step;

        constexpr Iterator(s64 i, s64 step = 1) : I(i), Step(step) {}

        operator s32() const { return (s32) I; }
        operator s64() const { return I; }

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

        constexpr bool operator==(const Iterator &other) const { return Step < 0 ? (I <= other.I) : (I >= other.I); }
        constexpr bool operator!=(const Iterator &other) const { return Step < 0 ? (I > other.I) : (I < other.I); }
    };

    Iterator Begin;
    Iterator End;

   public:
    constexpr range(u64 stop) : range(0, stop, 1) {}
    constexpr range(s64 start, s64 stop) : range(start, stop, 1) {}
    constexpr range(s64 start, s64 stop, s64 step) : Begin(start, step), End(stop) {}

    // Checks if a value is inside the given range.
    // This also accounts for stepping.
    constexpr bool has(s64 value) const {
        if (Begin.Step > 0 ? (value >= Begin.I && value < End.I) : (value > End.I && value <= Begin.I)) {
            s64 diff = value - Begin.I;
            if (diff % Begin.Step == 0) {
                return true;
            }
        }
        return false;
    }

    constexpr Iterator begin() const { return Begin; }
    constexpr Iterator end() const { return End; }
};

LSTD_END_NAMESPACE
