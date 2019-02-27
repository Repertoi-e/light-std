#pragma once

#include "collector.hpp"

#include "../../vendor/FastDelegate/FastDelegate.hpp"

LSTD_BEGIN_NAMESPACE

namespace internal {

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
}  // namespace internal

template <typename Signature,
          typename Collector = Collector_Default<typename fastdelegate::FastDelegate<Signature>::ReturnType>>
struct Signal;

template <typename R, typename... Args, typename Collector>
struct Signal<R(Args...), Collector> : public NonCopyable {
   protected:
    using result_type = R;
    using callback_type = typename fastdelegate::FastDelegate<R(Args...)>;
    using collector_result_type = typename Collector::result_type;

   private:
    // Signal_Link implements a doubly-linked ring with ref-counted nodes containing the callbacks.
    struct Signal_Link {
        Signal_Link *Next = null, *Prev = null;
        callback_type Callback;
        s32 RefCount;

        Signal_Link() : Callback() {}
        ~Signal_Link() { assert(RefCount == 0); }

        void set_callback(const callback_type &cb) {
            Callback = cb;
            RefCount = 1;
        }

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
            Callback = typename callback_type();
            if (Next) Next->Prev = Prev;
            if (Prev) Prev->Next = Next;
            return decref();
        }

        size_t add_before(const callback_type &cb, const Allocator_Closure &allocator) {
            Signal_Link *link = New<Signal_Link>(allocator);
            link->set_callback(cb);

            link->Prev = Prev;  // link to last
            link->Next = this;
            Prev->Next = link;  // link from last
            Prev = link;

            return (size_t) link;
        }

        std::pair<Signal_Link *, bool> remove_sibling(size_t id, const Allocator_Closure &allocator) {
            for (Signal_Link *link = this->Next ? this->Next : this; link != this; link = link->Next)
                if (id == (size_t) link) {
                    return {link, link->unlink()};
                }
            return {null, false};
        }

        static void operator delete(void *ptr, std::size_t sz) {
            assert(false && "Wtf?");
        }
    };

   public:
    // The allocator used for connecting new callbacks.
    // This value is null until this object allocates memory or the user sets it manually.
    Allocator_Closure Allocator;

    Signal_Link *CallbackRing = null;

    internal::Collector_Invocation<Collector, R(Args...)> Invoker;

    // Connects default callback if non-nullptr.
    Signal(const callback_type &cb = null) {
        if (!cb.empty()) {
            ensure_ring();
            CallbackRing->Callback = cb;
        }
    }

    ~Signal() { release(); }

    // Releases all memory associated with this signal
    void release() {
        if (CallbackRing) {
            while (CallbackRing->Next != CallbackRing) {
                auto result = CallbackRing->Next->unlink();
                if (result) {
                    Delete(CallbackRing->Next, Allocator);
                }
            }
            assert(CallbackRing->RefCount >= 2);
            CallbackRing->decref();
            auto result = CallbackRing->decref();
            assert(result);
            Delete(CallbackRing, Allocator);
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
        if (!CallbackRing) return false;
        auto [link, result] = CallbackRing->remove_sibling(connection, Allocator);
        if (result) {
            Delete(link, Allocator);
        }
        return result;
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
            if (link->Callback != null) {
                if (!Invoker.invoke(collector, link->Callback, std::forward<Args>(args)...)) break;
            }
            Signal_Link *old = link;
            link = old->Prev;
            link->incref();
            assert(!old->decref());
        } while (link != CallbackRing);
        assert(!link->decref());

        return collector.result();
    }

   private:
    void ensure_ring() {
        if (!CallbackRing) {
            CallbackRing = New_and_ensure_allocator<Signal_Link>(Allocator);
            CallbackRing->set_callback(callback_type());
            CallbackRing->incref();  // set RefCount = 2, head of ring, can be deactivated but not removed
            CallbackRing->Next = CallbackRing;
            CallbackRing->Prev = CallbackRing;
        }
    }
};

LSTD_END_NAMESPACE