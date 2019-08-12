#pragma once

/// Defines the graphics API that can be used both by the engine and the game.
/// Implementations of it can be switched dynamically.

#include "window.h"

namespace le {

struct graphics {
    virtual ~graphics() = default;

    virtual void init(window *targetWindow) = 0;

    virtual void clear_color(vec4 color) = 0;

    virtual void set_blend(bool enabled) {}
    virtual void set_depth_testing(bool enabled) {}

    virtual void swap() = 0;

    virtual void release() = 0;
};

/*
struct software_graphics : graphics {};
struct gl_graphics : graphics {};
struct vulkan_graphics : graphics {};
struct metal_graphics : graphics {};
*/

struct d3d_graphics : graphics {
    // Any state that must be saved in the implementation
    char D3DData[256]{};

    d3d_graphics() = default;
    ~d3d_graphics() { release(); }

    void init(window *targetWindow) override;

    void clear_color(vec4 color) override;

    void set_blend(bool enabled) override;
    void set_depth_testing(bool enabled) override;

    void swap() override;

    void release() override;

   private:
    void change_size(const window_resized_event &e);
};
}  // namespace le
