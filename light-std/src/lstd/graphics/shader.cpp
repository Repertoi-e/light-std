#include "shader.h"

#include "../file/handle.h"
#include "api.h"

LSTD_BEGIN_NAMESPACE

extern shader::impl g_D3DShaderImpl;  // Defined in d3d_shader.cpp
void shader::init(graphics *g, const file::handle &fileHandle) {
    clone(&FilePath, fileHandle.Path);

    auto [sucess, source] = fileHandle.read_entire_file();
    if (!sucess) return;

    Graphics = g;
    Source = source;

    if (g->API == graphics_api::Direct3D) {
        Impl = g_D3DShaderImpl;
    } else {
        assert(false);
    }
    Impl.Init(this);
}

void shader::init(graphics *g, const string &source) {
    Graphics = g;

    clone(&Source, source);

    if (g->API == graphics_api::Direct3D) {
        Impl = g_D3DShaderImpl;
    } else {
        assert(false);
    }
    Impl.Init(this);
}

void shader::bind() { Impl.Bind(this); }
void shader::unbind() { Impl.Unbind(this); }

void shader::release() {
    if (Impl.Release) Impl.Release(this);
}

LSTD_END_NAMESPACE
