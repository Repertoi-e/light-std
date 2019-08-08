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
struct Collector_Until0 {
    using result_t = Result;
    result_t Last;

    bool operator()(Result r) {
        Last = r;
        return Last ? true : false;
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

// Returns the result of the all signal handlers from a signal emission in a Dynamic_Array
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
   protected:
    using result_t = R;
    using callback_t = delegate<R(Args...)>;
    using collector_result_t = typename Collector::result_t;

   private:
    // Signal_Link implements a doubly-linked ring with ref-counted nodes containing the callbacks.
    struct signal_link {
        signal_link *Next = null, *Prev = null;
        callback_t Callback;
        s32 RefCount = 1;

        signal_link(const callback_t &cb) : Callback(cb) {}
        ~signal_link() { assert(RefCount == 0); }

        void incref() {
            RefCount += 1;
            assert(RefCount > 0);
        }

        // Returns whether it should be destroyed
        bool decref() {
            RefCount -= 1;
            if (!RefCount) {
                return true;
            }
            assert(RefCount > 0);
            return false;
        }

        // Returns whether it should be destroyed
        bool unlink() {
            Callback = callback_t();
            if (Next) Next->Prev = Prev;
            if (Prev) Prev->Next = Next;
            return decref();
        }

        size_t add_before(const callback_t &cb, const allocator &allocator) {
            signal_link *link = new (allocator) signal_link(cb);
            link->Prev = Prev;  // link to last
            link->Next = this;
            Prev->Next = link;  // link from last
            Prev = link;

            return (size_t) link;
        }

        pair<signal_link *, bool> remove_sibling(size_t id, const allocator &allocator) {
            for (signal_link *link = this->Next ? this->Next : this; link != this; link = link->Next)
                if (id == (size_t) link) {
                    return {link, link->unlink()};
                }
            return {null, false};
        }
    };

   public:
    // The allocator used for connecting new callbacks.
    // This value is null until this object allocates memory or the user sets it manually.
    allocator Allocator;
    signal_link *CallbackRing = null;

    internal::collector_invocation<Collector, R(Args...)> Invoker;

    // Connects default callback if non-null.
    signal(const callback_t &cb = null) {
        if (cb) {
            ensure_ring();
            CallbackRing->Callback = cb;
        }
    }

    ~signal() { release(); }

    // Releases all memory associated with this signal
    void release() {
        if (CallbackRing) {
            while (CallbackRing->Next != CallbackRing) {
                auto next = CallbackRing->Next;
                if (next->unlink()) delete next;
            }
            assert(CallbackRing->RefCount >= 2);
            CallbackRing->decref();
            assert(CallbackRing->decref());
            delete CallbackRing;
        }
    }

    // Add a new callback, returns a handler ID which you can use to remove the callback later
    template <typename... CBArgs>
    size_t connect(const callback_t &cb) {
        ensure_ring();
        return CallbackRing->add_before(cb, Allocator);
    }

    // Remove a callback via connection id. Returns true on success.
    bool disconnect(size_t connection) {
        if (!CallbackRing) return false;
        auto [link, result] = CallbackRing->remove_sibling(connection, Allocator);
        if (result) {
            delete link;
        }
        return result;
    }

    // Emit a signal, i.e. invoke all callbacks and collect return types with the Collector.
    // Stores the result in _out_.
    // _out_ must be null if the result type is void, in other cases if _out_ is null the result is just ignored.
    void emit(collector_result_t *out, Args... args) {
        Collector collector;
        if (!CallbackRing) {
            if constexpr (!is_same_v<collector_result_t, void>) {
                if (out) clone(out, collector.result());
            }
            return;
        }

        signal_link *link = CallbackRing;
        link->incref();
        do {
            if (link->Callback != null) {
                if (!Invoker.invoke(collector, link->Callback, ((Args &&) args)...)) break;
            }
            signal_link *old = link;
            link = old->Prev;
            link->incref();
            assert(!old->decref());
        } while (link != CallbackRing);
        assert(!link->decref());

        if constexpr (!is_same_v<collector_result_t, void>) {
            if (out) clone(out, collector.result());
        }
    }

   private:
    void ensure_ring() {
        if (!CallbackRing) {
            CallbackRing = new (&Allocator) signal_link(callback_t());
            CallbackRing->incref();  // set RefCount = 2, head of ring, can be deactivated but not removed
            CallbackRing->Next = CallbackRing;
            CallbackRing->Prev = CallbackRing;
        }
    }
};

LSTD_END_NAMESPACE