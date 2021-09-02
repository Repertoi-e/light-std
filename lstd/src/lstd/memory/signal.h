#pragma once

#include "array.h"

LSTD_BEGIN_NAMESPACE

// Returns the result of the last callback from a signal emission.
template <typename Result>
struct collector_last {
    using result_t = Result;
    result_t Last;

    bool operator()(Result r) {
        Last = r;
        return true;
    }

    const result_t &result() { return Last; }
};

template <typename Result>
struct collector_default : collector_last<Result> {
};

// Specialization for signals with void return type.
template <>
struct collector_default<void> {
    using result_t = void;

    void result() const {
    }

    bool operator()() const { return true; }
};

// Keep signal emissions going while callbacks return !0 (true).
template <typename Result>
struct collector_until0 {
    using result_t = Result;
    result_t Last;

    bool operator()(Result r) {
        Last = r;
        return Last ? true : false;
    }

    const result_t &result() { return Last; }
};

template <typename Result, Result target>
struct collector_until {
    using result_t = Result;
    result_t Last;

    bool operator()(Result r) {
        Last = r;
        return Last == target ? false : true;
    }

    const result_t &result() { return Last; }
};

// Keep signal emissions going while callbacks return 0 (false).
template <typename Result>
struct collector_while0 {
    using result_t = Result;
    result_t Last;

    bool operator()(Result r) {
        Last = r;
        return Last ? false : true;
    }

    const result_t &result() { return Last; }
};

template <typename Result, Result target>
struct collector_while {
    using result_t = Result;
    result_t Last = target;

    bool operator()(Result r) {
        Last = r;
        return Last == target ? true : false;
    }

    const result_t &result() { return Last; }
};

// Returns the result of the all signal handlers from a signal emission in an array
template <typename Result>
struct collector_array {
    using result_t = array<Result>;
    result_t Array;

    bool operator()(Result r) {
        array_append(Array, r);
        return true;
    }

    const result_t &result() { return Array; }
};

namespace internal {

// Collector_Invocation invokes callbacks differently depending on return type.
template <typename, typename>
struct collector_invocation;

// Specialization for regular signals.
template <typename Collector, typename R, typename... Args>
struct collector_invocation<Collector, R(Args ...)> {
    bool invoke(Collector &collector, const delegate<R(Args ...)> &cb, Args ... args) { return collector(cb(args...)); }
};

// Specialization for signals with void return type.
template <typename Collector, typename... Args>
struct collector_invocation<Collector, void(Args ...)> {
    bool invoke(Collector &collector, const delegate<void(Args ...)> &cb, Args ... args) {
        cb(args...);
        return collector();
    }
};
} // namespace internal

template <typename Signature, typename Collector = collector_default<typename delegate<Signature>::return_t>>
struct signal;

template <typename R, typename... Args, typename Collector>
struct signal<R(Args ...), Collector> : public non_copyable {
    using result_t = R;
    using callback_t = delegate<R(Args ...)>;
    using collector_result_t = typename Collector::result_t;

    array<callback_t> Callbacks;
    internal::collector_invocation<Collector, R(Args ...)> Invoker;

    bool CurrentlyEmitting = false;
    array<s64> ToRemove;

    // Connects default callback if non-null.
    signal(const callback_t &cb = null) {
        if (cb) array_append(Callbacks, cb);
    }

    // We no longer use destructors for deallocation.
    // ~signal() { release(); }

    void release() {
        free(Callbacks);
        free(ToRemove);
    }

    // Add a new callback, returns a handler ID which you can use to remove the callback later
    template <typename... CBArgs>
    s64 connect(const callback_t &cb) {
        if (cb) array_append(Callbacks, cb);
        return Callbacks.Count - 1;
    }

    // Remove a callback via connection id. Returns true on success.
    bool disconnect(s64 index) {
        if (!CurrentlyEmitting) {
            assert(index <= Callbacks.Count);
            if (Callbacks[index]) {
                Callbacks[index] = null;
                return true;
            }
            return false;
        }
        array_append(ToRemove, index);
        return false; // We will remove the callback once we have finished emitting
    }

    // Emit a signal, i.e. invoke all callbacks and collect return types with the Collector.
    // If the result is an array, the caller is responsible for freeing the memory.
    //
    // [[nodiscard]] to issue a warning if a leak happens because the caller ignored the return value.
    // This library follows the convention that if the function is marked as [[nodiscard]], the returned value should be freed.
    [[nodiscard]] collector_result_t emit(Args ... args) {
        CurrentlyEmitting = true;
        Collector collector;
        For(Callbacks) {
            if (it && !Invoker.invoke(collector, it, (Args &&) args...)) break;
        }
        CurrentlyEmitting = false;

        For(ToRemove) {
            assert(it <= Callbacks.Count);
            if (Callbacks[it]) Callbacks[it] = null;
        }
        array_reset(ToRemove);

        if constexpr (!types::is_same<collector_result_t, void>) {
            return collector.result();
        }
    }
};

LSTD_END_NAMESPACE
