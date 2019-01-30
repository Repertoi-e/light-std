#pragma once

#include "../core.hpp"

namespace le {

// The Application has layers that are stacked on top of each other.
// Layers receive events from top to bottom, and get updated from bottom to top.
// Subclass this and implement desired functionality,
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
class LE_API Layer {
public:
    Layer() {}
    virtual ~Layer(){};

    // Gets called when the layer is pushed onto the layer stack
    virtual void on_add() {}

    // Gets called when the layer is removed from the layer stack
    virtual void on_remove() {}

    // Gets called 60 times per frame, update and render logic happen here
    // dt is const (may be 1/60 or whatever the monitor's refresh Hz is)
    virtual void on_update(f32 dt) {}
};
}  // namespace le