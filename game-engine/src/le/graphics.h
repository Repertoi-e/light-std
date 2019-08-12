#pragma once

/// Defines the graphics API that can be used both by the engine and the game.
/// Implementations of it can be switched dynamically.

struct graphics {
    graphics() = default;
    virtual ~graphics() = default;

    virtual bool init() = 0;
};

/*
struct software_graphics : graphics {};
struct gl_graphics : graphics {};
struct vulkan_graphics : graphics {};
struct metal_graphics : graphics {};
*/

struct d3d_graphics : graphics {
    d3d_graphics() = default;
    ~d3d_graphics() = default;

    bool init() override;
};
