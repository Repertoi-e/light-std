#pragma once

#include "../core.hpp"

namespace le {

inline void layer_do_nothing(void *data) {}
inline void layer_do_nothing(void *data, f32 dt) {}

// The Application has layers that are stacked on top of each other.
// Layers receive events from top to bottom, and get updated from bottom to top.
// Subclass this and provide desired function via function pointers,
// then push it to the layer stack in Application.
//
// Use on_add() and on_remove() instead of ctor and dtor because that way you
// don't have to destruct and construct the whole object again if you want to
// remove it from the stack.
// Basically on_remove() should reset the state of your layer and make it ready
// for a call to on_add().
//
// !!! You should connect to signals for events in the on_add() function of your layer
// and ALWAYS remove them in on_remove() (unless you are sure your layer will not get removed or destroyed)
//
// By default the function pointers aren't null, but a pointer to a function that does nothing.
// This is so you aren't required to provide every function if it isn't needed in your case.
class LE_API layer {
   public:
    using on_add_type = void (*)(void *data);
    using on_remove_type = void (*)(void *data);
    using on_update_type = void (*)(void *data, f32 dt);

    // Gets called when the layer is pushed onto the layer stack
    on_add_type on_add_function = layer_do_nothing;

    // Gets called when the layer is removed from the layer stack
    on_remove_type on_remove_function = layer_do_nothing;

    // Gets called 60 times per frame, update and render logic happen here
    // dt is const (may be 1/60 or whatever the monitor's refresh Hz is)
    on_update_type on_update_function = layer_do_nothing;

    void on_add() { on_add_function(this); }
    void on_remove() { on_remove_function(this); }
    void on_update(f32 dt) { on_update_function(this, dt); }
};
}  // namespace le