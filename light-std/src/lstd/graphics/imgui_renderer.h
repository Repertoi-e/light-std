#pragma once

#include "api.h"
#include "../../vendor/imgui/imgui.h"

LSTD_BEGIN_NAMESPACE

struct imgui_renderer {
    graphics *Graphics = null;

    buffer VB, IB, UB;
    texture_2D FontTexture;
    shader Shader;
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
