#include "texture.h"

#include "../memory/pixel_buffer.h"
#include "api.h"

LSTD_BEGIN_NAMESPACE

extern texture_2D::impl g_D3DTexture2DImpl;  // Defined in d3d_texture.cpp
void texture_2D::init(graphics *g, s32 width, s32 height, texture_filter filter, texture_wrap wrap) {
    Graphics = g;

    Width = width;
    Height = height;

    Filter = filter;
    Wrap = wrap;

    if (g->API == graphics_api::Direct3D) {
        Impl = g_D3DTexture2DImpl;
    } else {
        assert(false);
    }
    Impl.Init(this);
}

void texture_2D::init_as_render_target(graphics *g, s32 width, s32 height, texture_filter filter, texture_wrap wrap) {
    RenderTarget = true;
    init(g, width, height, filter, wrap);
}

void texture_2D::set_data(pixel_buffer data) { Impl.SetData(this, data); }

void texture_2D::bind(u32 slot) {
    BoundSlot = slot;
    Impl.Bind(this);
}

void texture_2D::unbind() {
    Impl.Unbind(this);
    BoundSlot = (u32) -1;
}

void texture_2D::release() {
    if (Impl.Release) Impl.Release(this);
}

LSTD_END_NAMESPACE
