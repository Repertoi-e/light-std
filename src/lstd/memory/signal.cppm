module;

#include "../common.h"

export module lstd.signal;

import lstd.array;
import lstd.delegate;

LSTD_BEGIN_NAMESPACE

template <typename Signature>
struct signal;

template <typename R, typename... Args>
struct signal<R(Args...)> {
    using result_t   = R;
    using callback_t = delegate<R(Args...)>;

    // @ThreadSafety: Lock-free linked list?
    array<callback_t> Callbacks;

    bool CurrentlyEmitting = false;
    array<s64> ToRemove;
};

// Add a new callback, returns a handler ID which you can use to remove the callback later
template <typename F>
s64 connect(signal<F> *s, delegate<F> cb) {
    if (cb) add(&s->Callbacks, cb);
    return s->Callbacks.Count - 1;
}

// Remove a callback via connection id. Returns true on success.
template <typename F>
bool disconnect(signal<F> *s, s64 index) {
    if (!s->CurrentlyEmitting) {
        assert(index <= s->Callbacks.Count);
        if (s->Callbacks[index]) {
            s->Callbacks[index] = null;
            return true;
        }
        return false;
    }
    add(&s->ToRemove, index);
    return true;  // We will remove the callback once we have finished emitting
}

template <typename T>
void handle_to_remove_after_emit_end(signal<T> *s) {
    For(s->ToRemove) {
        assert(it <= s->Callbacks.Count);
        if (s->Callbacks[it]) s->Callbacks[it] = null;
    }
    reset(&s->ToRemove);
}

// Emits to all callbacks
template <typename R, typename... Args>
void emit(signal<R(Args...)> *s, Args ref... args) {
    s->CurrentlyEmitting = true;
    For(s->Callbacks) if (it) it((Args ref) args...);
    s->CurrentlyEmitting = false;
    handle_to_remove_after_emit_end(s);
}

// Calls registered callbacks until one returns true.
// Used for e.g. window events - when the user clicks on the UI the event should not be propagated to the world.
template <typename R, typename... Args>
void emit_while_false(signal<R(Args...)> *s, Args... args) {
    static_assert(types::is_convertible<R, bool>);

    s->CurrentlyEmitting = true;
    For(s->Callbacks) if (it) if (it((Args ref) args...)) break;
    s->CurrentlyEmitting = false;
    handle_to_remove_after_emit_end(s);
}

// Calls registered callbacks until one returns false
template <typename R, typename... Args>
void emit_while_true(signal<R(Args...)> *s, Args... args) {
    static_assert(types::is_convertible<R, bool>);

    s->CurrentlyEmitting = true;
    For(s->Callbacks) if (it) if (!it((Args ref) args...)) break;
    s->CurrentlyEmitting = false;
    handle_to_remove_after_emit_end(s);
}

template <typename T>
void free_signal(signal<T> *s) {
    // @Cleanup Make it a stack array
    if (s->Callbacks) free(s->Callbacks.Data);
    if (s->ToRemove) free(s->ToRemove.Data);
}

LSTD_END_NAMESPACE
