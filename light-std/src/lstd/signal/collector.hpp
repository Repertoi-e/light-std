#pragma once

#include "../memory/dynamic_array.hpp"

LSTD_BEGIN_NAMESPACE

// Returns the result of the last callback from a signal emission.
template <typename Result>
struct Collector_Last {
    using result_type = Result;

    inline bool operator()(Result r) {
        Last = r;
        return true;
    }
    const result_type &result() { return Last; }

   private:
    result_type Last;
};

template <typename Result>
struct Collector_Default : Collector_Last<Result> {};

// Specialization for signals with void return type.
template <>
struct Collector_Default<void> {
    using result_type = void;

    void result() {}
    inline bool operator()(void) { return true; }
};

// Keep signal emissions going while callbacks return !0 (true).
template <typename Result>
struct Collector_Until0 {
    using result_type = Result;

    inline bool operator()(Result r) {
        Last = r;
        return Last ? true : false;
    }
    const result_type &result() { return Last; }

   private:
    result_type Last;
};

// Keep signal emissions going while callbacks return 0 (false).
template <typename Result>
struct Collector_While0 {
    using result_type = Result;

    inline bool operator()(Result r) {
        Last = r;
        return Last ? false : true;
    }

    const result_type &result() { return Last; }

   private:
    result_type Last;
};

// Returns the result of the all signal handlers from a signal emission in a Dynamic_Array
template <typename Result>
struct Collector_Array {
    using result_type = Dynamic_Array<Result>;

    inline bool operator()(Result r) {
        Array.add(r);
        return true;
    }

    const result_type &result() { return Array; }

   private:
    result_type Array;
};

LSTD_END_NAMESPACE