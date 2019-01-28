#pragma once

#include "collector.hpp"

#include "../../vendor/FastDelegate/FastDelegate.hpp"

LSTD_BEGIN_NAMESPACE

namespace internal {

template <typename, typename>
struct Proto_Signal;

// Collector_Invocation invokes callbacks differently depending on return type.
template <typename, typename>
struct Collector_Invocation;

// Specialization for regular signals.
template <class Collector, class R, class... Args>
struct Collector_Invocation<Collector, R(Args...)> {
    inline bool invoke(Collector &collector, const fastdelegate::FastDelegate<R(Args...)> &cb, Args... args) {
        return collector(cb(args...));
    }
};

// Specialization for signals with void return type.
template <class Collector, class... Args>
struct Collector_Invocation<Collector, void(Args...)> {
    inline bool invoke(Collector &collector, const fastdelegate::FastDelegate<void(Args...)> &cb, Args... args) {
        cb(args...);
        return collector();
    }
};

template <typename Collector, typename R, typename... Args>
struct Proto_Signal<R(Args...), Collector> : private Collector_Invocation<Collector, R(Args...)>, public NonCopyable {
   protected:
    using callback_type = fastdelegate::FastDelegate<R(Args...)>;
    using result_type = R;
    using collector_result_type = typename Collector::result_type;

   private:
    // Signal_Link implements a doubly-linked ring with ref-counted nodes containing the callbacks.
    struct Signal_Link {
        Signal_Link *Next = null, *Prev = null;
        callback_type Callback;
        s32 RefCount;

        Signal_Link() : Callback() {}
        Signal_Link(const callback_type &cbf) : Callback(cbf), RefCount(1) {}
        ~Signal_Link() { assert(RefCount == 0); }

        void incref() {
            RefCount += 1;
            assert(RefCount > 0);
        }

        // Takes an allocator in case it's time to get destroyed
        void decref(const Allocator_Closure &allocator) {
            RefCount -= 1;
            if (!RefCount) {
                Delete(this, allocator);
            } else {
                assert(RefCount > 0);
            }
        }

        // Takes an allocator in case it's time to get destroyed
        void unlink(const Allocator_Closure &allocator) {
            Callback = typename callback_type();
            if (Next) Next->Prev = Prev;
            if (Prev) Prev->Next = Next;
            decref(allocator);
        }

        size_t add_before(const callback_type &cb, const Allocator_Closure &allocator) {
            Signal_Link *link = New<Signal_Link>(allocator);
            new (link) Signal_Link(cb);

            link->Prev = Prev;  // link to last
            link->Next = this;
            Prev->Next = link;  // link from last
            Prev = link;

            return (size_t) link;
        }

        bool remove_sibling(size_t id, const Allocator_Closure &allocator) {
            for (Signal_Link *link = this->Next ? this->Next : this; link != this; link = link->Next)
                if (id == (size_t) link) {
                    link->unlink(allocator);
                    return true;
                }
            return false;
        }
    };

   public:
    // The allocator used for connecting new callbacks.
    // This value is null until this object allocates memory or the user sets it manually.
    Allocator_Closure Allocator;

    Signal_Link *CallbackRing = null;

    // Connects default callback if non-nullptr.
    Proto_Signal(const callback_type &cb) {
        if (!cb.empty()) {
            ensure_ring();
            CallbackRing->Callback = cb;
        }
    }

    ~Proto_Signal() { release(); }

    // Releases all memory associated with this signal
    void release() {
        if (CallbackRing) {
            while (CallbackRing->Next != CallbackRing) CallbackRing->Next->unlink(Allocator);
            assert(CallbackRing->RefCount >= 2);
            CallbackRing->decref(Allocator);
            CallbackRing->decref(Allocator);
        }
    }

    // Add a new callback, returns a handler ID which you can use to remove the callback later
    template <typename... CBArgs>
    size_t connect(const callback_type &cb) {
        ensure_ring();
        return CallbackRing->add_before(cb, Allocator);
    }

    // Remove a callback via connection id. Returns true on success.
    bool disconnect(size_t connection) {
        return CallbackRing ? CallbackRing->remove_sibling(connection, Allocator) : false;
    }

    // Emit a signal, i.e. invoke all callbacks and collect return types with the Collector
    collector_result_type emit(Args... args) {
        Collector collector;
        if (!CallbackRing) {
            return collector.result();
        }

        Signal_Link *link = CallbackRing;
        link->incref();
        do {
            if (link->Callback != nullptr) {
                if (!this->invoke(collector, link->Callback, std::forward<Args>(args)...)) break;
            }
            Signal_Link *old = link;
            link = old->Next;
            link->incref();
            old->decref(Allocator);
        } while (link != CallbackRing);
        link->decref(Allocator);

        return collector.result();
    }

   private:
    void ensure_ring() {
        if (!CallbackRing) {
            CallbackRing = New_and_ensure_allocator<Signal_Link>(Allocator);
            new (CallbackRing) Signal_Link(callback_type());

            CallbackRing->incref();  // RefCount = 2, head of ring, can be deactivated but not removed
            CallbackRing->Next = CallbackRing;
            CallbackRing->Prev = CallbackRing;
        }
    }
};
}  // namespace internal

template <typename Signature, typename Collector = Collector_Default<typename fastdelegate::FastDelegate<Signature>::ReturnType>>
struct Signal : internal::Proto_Signal<Signature, Collector> {
    using proto_signal = internal::Proto_Signal<Signature, Collector>;
    using callback_type = typename proto_signal::callback_type;

    Signal(const callback_type &cb = callback_type()) : proto_signal(cb) {}
};

LSTD_END_NAMESPACE