#pragma once

#include "../memory/array.h"
#include "../memory/string.h"
#include "gtype.h"

#if OS == WINDOWS
struct ID3D11Buffer;
struct ID3D11InputLayout;
#endif

LSTD_BEGIN_NAMESPACE

struct buffer_layout {
    struct element {
        string Name;
        gtype Type = gtype::Unknown;
        s64 SizeInBits = 0;
        bool Normalized = false;
        s64 Count = 0;
        u32 AlignedByteOffset = 0;  // 1-bit values add 7 bits of packing for the next element
    };

    array<element> Elements;
    s64 TotalSize = 0;  // Calculated in bytes (1-bit values add 7 bits of packing),
                        // generally used internally to calculate the offset for the next element

    void add(const string &name, gtype type, s64 count = 1, bool normalized = false);
    void add_padding(s64 bytes);

    void release() { Elements.release(); }
};

enum class primitive_topology { PointList = 0,
                                LineList,
                                LineStrip,
                                TriangleList,
                                TriangleStrip };

enum class buffer_type {
    None = 0,
    Vertex_Buffer,
    Index_Buffer,
    Shader_Uniform_Buffer  // Used to pack shader data, "constant buffers" (dx), "uniform buffer objects" (gl), etc.
};

// This only makes sense when using DX, OpenGL doesn't support these options when binding a buffer
enum class buffer_usage {
    Default,    // The buffer requires read and write access by the GPU
    Immutable,  // Cannot be modified after creation, so it must be created with initial data
    Dynamic,    // Can be written to by the CPU and read by the GPU
    Staging     // Supports data transfer (copy) from the GPU to the CPU
};

enum class buffer_map_access {
    Read,                    // Buffer can only be read by the CPU
    Write,                   // Buffer can only be written to by the CPU
    Read_Write,              // Buffer can be read and written to by the CPU
    Write_Discard_Previous,  // Previous contents of buffer may be discarded, and new buffer is opened for writing
    Write_Unsynchronized     // An advanced option that allows you to add more data to the buffer even while the GPU is
                             // using parts. However, you must not work with the parts the GPU is using.
};

struct graphics;

struct buffer : non_copyable, non_movable {
#if OS == WINDOWS
    struct {
        ID3D11Buffer *Buffer = null;
        ID3D11InputLayout *Layout = null;

        // Based on the definition of _D3D11_MAPPED_SUBRESOURCE_
        char MappedData[POINTER_SIZE + sizeof(u32) + sizeof(u32)]{};
    } D3D{};
#endif

    struct impl {
        void (*Init)(buffer *b, const char *data) = null;
        void (*SetInputLayout)(buffer *b, const buffer_layout &layout) = null;

        void *(*Map)(buffer *b, buffer_map_access access) = null;
        void (*Unmap)(buffer *b) = null;

        void (*Bind)(buffer *b, primitive_topology topology, u32 offset, u32 stride, shader_type shaderType, u32 position) = null;
        void (*Unbind)(buffer *b) = null;
        void (*Release)(buffer *b) = null;
    } Impl{};

    graphics *Graphics = null;

    buffer_type Type = buffer_type::None;
    buffer_usage Usage = buffer_usage::Default;
    s64 Size = 0;
    s64 Stride = 0;  // Determined by the buffer layout

    buffer() = default;

    // We no longer use destructors for deallocation.
    // ~buffer() { release(); }

    void init(graphics *g, buffer_type type, buffer_usage usage, s64 size, const char *data = null);

    void set_input_layout(const buffer_layout &layout);

    void *map(buffer_map_access access);
    void unmap();

    void bind_vb(primitive_topology topology, u32 offset = 0, u32 customStride = 0);
    void bind_ib(u32 offset = 0);
    void bind_ub(shader_type shaderType, u32 position);

    void unbind();

    void release();
};

LSTD_END_NAMESPACE
