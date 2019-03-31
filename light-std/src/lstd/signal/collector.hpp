#pragma once

#include "../memory/dynamic_array.hpp"

LSTD_BEGIN_NAMESPACE

// Returns the result of the last callback from a signal emission.
template <typename Result>
struct collector_last {
    using result_t = Result;

    bool operator()(Result r) {
        _Last = r;
        return true;
    }
    const result_t &result() { return _Last; }

   private:
    result_t _Last;
};

template <typename Result>
struct collector_default : collector_last<Result> {};

// Specialization for signals with void return type.
template <>
struct collector_default<void> {
    using result_t = void;

    void result() const {}
    bool operator()() const { return true; }
};

// Keep signal emissions going while callbacks return !0 (true).
template <typename Result>
struct Collector_Until0 {
    using result_t = Result;

    bool operator()(Result r) {
        _Last = r;
        return _Last ? true : false;
    }
    const result_t &result() { return _Last; }

   private:
    result_t _Last;
};

// Keep signal emissions going while callbacks return 0 (false).
template <typename Result>
struct collector_while0 {
    using result_t = Result;

    bool operator()(Result r) {
        _Last = r;
        return _Last ? false : true;
    }

    const result_t &result() { return _Last; }

   private:
    result_t _Last;
};

// Returns the result of the all signal handlers from a signal emission in a Dynamic_Array
template <typename Result>
struct collector_array {
    using result_t = dynamic_array<Result>;

    bool operator()(Result r) {
        _Array.add(r);
        return true;
    }

    const result_t &result() { return _Array; }

   private:
    result_t _Array;
};

LSTD_END_NAMESPACE