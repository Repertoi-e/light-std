#pragma once

#include "layer.hpp"

#include "../window/window.hpp"

#include <lstd/memory/dynamic_array.hpp>

namespace le {

// The application works with layers.
// e.g.
// [layer-1] [l-2] [l-3]
//
// Updating happens from l-1 to l-3, but events are emitted in reverse: from l-3 to l-1
// You should connect to signals for events in the on_add() function of your layer
struct LE_API application {
    window *Window;

    application() { s_Instance = this; }

    // This is called from the entry point and starts the main game loop
    // Implementation in *_application.cpp (platform-specific)
    void run();

    layer *add_layer(layer *layer) {
        _Layers.add(layer);
        layer->on_add();
        return layer;
    }

    bool remove_layer(layer *layer) {
        size_t index = _Layers.find(layer);
        if (index == npos) return false;
        layer->on_remove();
        _Layers.remove(_Layers.begin() + index);
        return true;
    }

    static application &get() { return *s_Instance; }

   private:
    dynamic_array<layer *> _Layers;

    static application *s_Instance;
};

// Should be defined in the client program.
// When creating the application you should set the Window before returning it.
// Do not define int main() or any other entry point in your program (include "entry_point.hpp" instead)
application *create_application();

}  // namespace le