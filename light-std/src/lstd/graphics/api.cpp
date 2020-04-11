#include "api.h"

LSTD_BEGIN_NAMESPACE

extern graphics::impl g_D3DImpl;  // Defined in d3d_api.cpp

void graphics::init(graphics_api api) {
    API = api;
    if (api == graphics_api::Direct3D) {
        Impl = g_D3DImpl;
    } else {
        assert(false);
    }
    Impl.Init(this);

    if (TargetWindows.find([](auto x) { return !x.Window; }) == npos) TargetWindows.append();  // Add a null target
    set_target_window(null);
}

extern buffer::impl g_D3DBufferImpl;  // Defined in d3d_buffer.cpp

void buffer::init(graphics *g, buffer_type type, buffer_usage usage, size_t size, const char *data) {
    Graphics = g;
    Type = type;
    Usage = usage;
    Size = size;
    if (g->API == graphics_api::Direct3D) {
        Impl = g_D3DBufferImpl;
    } else {
        assert(false);
    }
    Impl.Init(this, data);
}

extern shader::impl g_D3DShaderImpl;  // Defined in d3d_shader.cpp

void shader::init(graphics *g, file::handle fileHandle) {
    clone(&FilePath, fileHandle.Path);

    string source;
    if (!fileHandle.read_entire_file(&source)) return;

    init(g, source);
}

void shader::init(graphics *g, string source) {
    Graphics = g;
    
	clone(&Source, source);

    if (g->API == graphics_api::Direct3D) {
        Impl = g_D3DShaderImpl;
    } else {
        assert(false);
    }
    Impl.Init(this);
}

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

void texture_2D::init_as_render_target(graphics *g, s32 width, s32 height, texture_filter filter,
                                       texture_wrap wrap) {
    RenderTarget = true;
    init(g, width, height, filter, wrap);
}

LSTD_END_NAMESPACE
