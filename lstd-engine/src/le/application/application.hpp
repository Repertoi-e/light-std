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
struct LE_API Application {
    Window *WindowPtr;

    Application::Application() { s_Instance = this; }

    // This is called from the entry point and starts the main game loop
    // Implementation in *_application.cpp (platform-specific)
    void run();

    Layer *add_layer(Layer *layer) {
        Layers.add(layer);
        layer->on_add();
        return layer;
    }

    bool remove_layer(Layer *layer) {
        size_t index = Layers.find(layer);
        if (index == npos) return false;
        layer->on_remove();
        Layers.remove(Layers.begin() + index);
        return true;
    }

    inline static Application &get() { return *s_Instance; }

   private:
    Dynamic_Array<Layer *> Layers;

    static Application *s_Instance;
};

// Should be defined in the client program.
// When creating the application you should allocate it with
// the default global allocator and set the WindowPtr before returning it.
// Do not define int main() or any other entry point in your program (include "entry_point.hpp" instead)
Application *create_application();

}  // namespace le