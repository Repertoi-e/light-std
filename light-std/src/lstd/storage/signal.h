#pragma once

#include "array.h"
#include "delegate.h"

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
        Array.append(r);
        return true;
    }

    const result_t &result() { return Array; }
};

namespace internal {

// Collector_Invocation invokes callbacks differently depending on return type.
template <typename, typename>
struct collector_invocation;

// Specialization for regular signals.
template <class Collector, class R, class... Args>
struct collector_invocation<Collector, R(Args...)> {
    bool invoke(Collector &collector, const delegate<R(Args...)> &cb, Args... args) { return collector(cb(args...)); }
};

// Specialization for signals with void return type.
template <class Collector, class... Args>
struct collector_invocation<Collector, void(Args...)> {
    bool invoke(Collector &collector, const delegate<void(Args...)> &cb, Args... args) {
        cb(args...);
        return collector();
    }
};
}  // namespace internal

template <typename Signature, typename Collector = collector_default<typename delegate<Signature>::return_t>>
struct signal;

template <typename R, typename... Args, typename Collector>
struct signal<R(Args...), Collector> : public non_copyable {
    using result_t = R;
    using callback_t = delegate<R(Args...)>;
    using collector_result_t = typename Collector::result_t;

    array<callback_t> Callbacks;
    internal::collector_invocation<Collector, R(Args...)> Invoker;

    // Connects default callback if non-null.
    signal(const callback_t &cb = null) {
        if (cb) clone(Callbacks.append(), cb);
    }

    ~signal() { release(); }
    void release() { Callbacks.release(); }

    // Add a new callback, returns a handler ID which you can use to remove the callback later
    template <typename... CBArgs>
    size_t connect(const callback_t &cb) {
        if (cb) clone(Callbacks.append(), cb);
        return Callbacks.Count - 1;
    }

    // Remove a callback via connection id. Returns true on success.
    bool disconnect(size_t index) {
        assert(index <= Callbacks.Count);
        if (Callbacks[index]) {
            Callbacks[index].release();
            return true;
        }
        return false;
    }

    // Emit a signal, i.e. invoke all callbacks and collect return types with the Collector.
    // Stores the result in _out_.
    // _out_ must be null if the result type is void, in other cases if _out_ is null the result is just ignored.
    void emit(collector_result_t *out, Args... args) {
        Collector collector;

        For(Callbacks) {
            if (it && !Invoker.invoke(collector, it, ((Args &&) args)...)) break;
        }

        if constexpr (!is_same_v<collector_result_t, void>) {
            if (out) clone(out, collector.result());
        }
    }
};

LSTD_END_NAMESPACE