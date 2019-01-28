#pragma once

#include "signal.hpp"

LSTD_BEGIN_NAMESPACE

// Resembles a queue of data you wish to emit to any slots registered to the event queue.
// Data can optionally be cleared when emitting events.
template <typename EventData, typename SlotSignature>
struct Event_Queue {
    using slot_type = Slot<SlotSignature>;

    // The queue of data in the event queue
    Dynamic_Array<EventData> Data;
    Signal<slot_type> Signal;
    
    Event_Queue() {}
    Event_Queue(const Event_Queue<EventData> &) = default;
    Event_Queue &operator=(const Event_Queue<EventData> &) = default;

    // Connects a slot to the queue
    template <typename... Args>
    void connect(Args &&... slot) {
        Signal.connect(std::forward<Args>(slot)...);
    }

    // Disconnects a slot from the queue
    template <typename... Args>
    void disconnect(Args &&... slot) {
        Signal.disconnect(std::forward<Args>(slot)...);
    }

    // This will not clear out the data you pushed to the Event_Queue
    void emit() const {
        For(Data) { Signal(it); }
    }

    // Emits out queued up events, and then clears the queued events.
    void emit_and_clear() {
        emit();
        Data.clear();
    }

    void operator()() { emit_and_clear(); }
    void operator()() const { emit(); }
};

LSTD_END_NAMESPACE