#pragma once

#include "../../dx_graphics.h"  // @TODO: Currently we use hardcoded dx_* objects, in the future provide a system to switch API during runtime
#include "imgui.h"

LSTD_BEGIN_NAMESPACE

struct imgui_renderer {
    graphics *Graphics = null;

    dx_buffer VB, IB, UB;
    dx_texture_2D FontTexture;
    dx_shader Shader;
    size_t VBSize = 0, IBSize = 0;

    imgui_renderer() = default;
    ~imgui_renderer() { release(); }

    void init(graphics *g);
    void draw(ImDrawData *drawData);
    void release();

   private:
    void set_render_state();
};

LSTD_END_NAMESPACE
