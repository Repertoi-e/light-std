#include "buffer.h"

#include "api.h"

LSTD_BEGIN_NAMESPACE

void buffer_layout::add(string name, gtype type, s64 count, bool normalized) {
    s64 sizeInBits = get_size_of_base_gtype_in_bits(type);

    count *= get_count_of_gtype(type);

    assert(TotalSize <= numeric_info<u32>::max());
    Elements.append({name, get_scalar_gtype(type), sizeInBits, normalized, count, (u32) TotalSize});

    if (sizeInBits == 1) sizeInBits = 8;
    TotalSize += (sizeInBits / 8) * count;
}

void buffer_layout::add_padding(s64 bytes) { TotalSize += bytes; }

extern buffer::impl g_D3DBufferImpl;  // Defined in d3d_buffer.cpp
void buffer::init(graphics *g, buffer_type type, buffer_usage usage, s64 size, const char *data) {
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

void buffer::set_input_layout(buffer_layout layout) { Impl.SetInputLayout(this, layout); }

void *buffer::map(buffer_map_access access) { return Impl.Map(this, access); }

void buffer::unmap() { Impl.Unmap(this); }

void buffer::bind_vb(primitive_topology topology, u32 offset, u32 customStride) {
    Impl.Bind(this, topology, offset, customStride, (shader_type) 0, 0);
}

void buffer::bind_ib(u32 offset) { Impl.Bind(this, (primitive_topology) 0, offset, 0, (shader_type) 0, 0); }

void buffer::bind_ub(shader_type shaderType, u32 position) {
    Impl.Bind(this, (primitive_topology) 0, 0, 0, shaderType, position);
}

void buffer::unbind() { Impl.Unbind(this); }

void buffer::release() {
    if (Impl.Release) Impl.Release(this);
}

LSTD_END_NAMESPACE
