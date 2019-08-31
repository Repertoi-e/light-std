#pragma once

#include "../graphics.h"
#include "imgui.h"

LSTD_BEGIN_NAMESPACE

struct imgui_renderer {
	graphics *Graphics;

    buffer *VB = null, *IB = null, *UB = null;
    texture_2D *FontTexture = null;
    shader *Shader = null;
    size_t VBSize = 5000, IBSize = 10000;

    imgui_renderer() = default;
    ~imgui_renderer() { release(); }

    void init(graphics *g);
    void draw(ImDrawData *drawData);
    void release();

   private:
	   void set_render_state();
};

LSTD_END_NAMESPACE
