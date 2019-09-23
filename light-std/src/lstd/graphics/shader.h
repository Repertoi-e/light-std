#pragma once

#include "../catalog.h"
#include "../storage/array.h"
#include "../storage/string.h"
#include "gtype.h"

#if OS == WINDOWS
struct ID3D11VertexShader;
struct ID3D11PixelShader;
#endif

LSTD_BEGIN_NAMESPACE

struct graphics;

// Holds both a vertex and a pixel shader (those are the two shader types we support for now!)
struct shader : public asset, non_copyable, non_movable {
#if OS == WINDOWS
    struct {
        ID3D11VertexShader *VS = null;
        ID3D11PixelShader *PS = null;
        void *VSBlob = null;  // We can't forward declare _ID3DBlob_
        void *PSBlob = null;
    } D3D{};
#endif

    struct impl {
        void (*Init)(shader *s) = null;
        void (*Bind)(shader *s) = null;
        void (*Unbind)(shader *s) = null;
        void (*Release)(shader *s) = null;
    } Impl{};

    graphics *Graphics;

    string Source;

    struct uniform {
        string Name;
        gtype Type = gtype::Unknown;

        size_t ByteSize = 0;
        size_t Offset = 0;
        size_t Count = 0;  // _Count_ * _Size_ gives the total size
    };

    struct uniform_buffer {
        string Name;
        size_t ByteSize = 0;
        array<uniform> Uniforms;

        shader_type ShaderType = shader_type::None;
        u32 Position = 0;
    };

    // We parse the shader source and extract metadata (not really sophisticated...)
    array<uniform_buffer> UniformBuffers;

    shader() = default;
    ~shader() { release(); }

    void init(graphics *g, file::path filePath);
    void init(graphics *g, string source);

    void bind() { Impl.Bind(this); }
    void unbind() { Impl.Unbind(this); }

    void release() {
        if (Impl.Release) Impl.Release(this);
    }
};

LSTD_END_NAMESPACE
