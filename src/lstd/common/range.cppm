module;

#include "../common/namespace.h"

export module lstd.range;

export import lstd.numeric;

LSTD_BEGIN_NAMESPACE

//
// Python-like range functionality
// e.g.
//
//  For(range(12)) { /*use the variable _it_ here*/ }   // [0, 12)
//  For(range(3, 10, 2)) {}								// every second integer (step 2) in [3, 10)
//  For(range(10, 0, -1)) {}							// reverse [10, 0)
// 
//    which is equivalent to:
//
//  For_as(it, range(12)) {}
//  For_as(it, range(3, 10, 2)) {}
//  For_as(it, range(10, 0, -1)) {}
//
//    which is equivalent to:
// 
//  for (int i = 0; i < 12; i++) {}
//  for (int i = 3; i < 10; i += 2) {}
//  for (int i = 10; i > 0; i += -1) {}
//
// 
// In release it gets optimized to no overhead.
//
export struct range {
	struct iterator {
		s64 I, Step;

		iterator(s64 i, s64 step = 1) : I(i), Step(step) {}

		operator s32() const { return (s32)I; }
		operator s64() const { return I; }

		s64 operator*() const { return I; }
		iterator operator++() { return I += Step, *this; }

		iterator operator++(s32) {
			iterator temp(*this);
			return I += Step, temp;
		}

		bool operator==(iterator other) const { return Step < 0 ? (I <= other.I) : (I >= other.I); }
		bool operator!=(iterator other) const { return Step < 0 ? (I > other.I) : (I < other.I); }
	};

	iterator Begin;
	iterator End;

	range(s64 start, s64 stop, s64 step) : Begin(start, step), End(stop) {}
	range(s64 start, s64 stop) : range(start, stop, 1) {}
	range(u64 stop) : range(0, stop, 1) {}

	// Checks if a value is inside the given range.
	// This also accounts for stepping.
	bool has(s64 value) const {
		if (Begin.Step > 0 ? (value >= Begin.I && value < End.I) : (value > End.I && value <= Begin.I)) {
			s64 diff = value - Begin.I;
			if (diff % Begin.Step == 0) {
				return true;
			}
		}
		return false;
	}

	iterator begin() const { return Begin; }
	iterator end() const { return End; }
};

LSTD_END_NAMESPACE
