#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/graphics/api.h"
#include "lstd/graphics/buffer.h"
#include "lstd/graphics/shader.h"
#include "lstd/os.h"

LSTD_BEGIN_NAMESPACE

void d3d_buffer_init(buffer *b, const char *data) {
    if (b->Usage == buffer_usage::Immutable) {
        assert(data && "Immutable buffers must be created with initial data");
    }

    D3D11_BUFFER_DESC desc;
    zero_memory(&desc, sizeof(desc));
    {
        assert(b->Size <= numeric_info<u32>::max());
        desc.ByteWidth = (u32) b->Size;

        if (b->Usage == buffer_usage::Immutable) desc.Usage = D3D11_USAGE_IMMUTABLE;
        if (b->Usage == buffer_usage::Dynamic) desc.Usage = D3D11_USAGE_DYNAMIC;
        if (b->Usage == buffer_usage::Staging) desc.Usage = D3D11_USAGE_STAGING;

        if (b->Type == buffer_type::Vertex_Buffer) desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        if (b->Type == buffer_type::Index_Buffer) desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        if (b->Type == buffer_type::Shader_Uniform_Buffer) desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        if (b->Usage == buffer_usage::Dynamic) desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (b->Usage == buffer_usage::Staging) desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    }

    D3D11_SUBRESOURCE_DATA srData;
    srData.pSysMem = data;
    srData.SysMemPitch = 0;
    srData.SysMemSlicePitch = 0;

    auto *srDataPtr = &srData;
    if (!data) srDataPtr = null;
    DX_CHECK(b->Graphics->D3D.Device->CreateBuffer(&desc, srDataPtr, &b->D3D.Buffer));
}

DXGI_FORMAT gtype_and_count_to_dxgi_format(gtype type, s64 count, bool normalized) {
    switch (type) {
        case gtype::BOOL:
            assert(count == 1);
            assert(!normalized);
            return DXGI_FORMAT_R1_UNORM;
        case gtype::U8:
            assert(count == 1);
            return normalized ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8_UINT;
        case gtype::S8:
            assert(count == 1);
            return normalized ? DXGI_FORMAT_R8_SNORM : DXGI_FORMAT_R8_SINT;
        case gtype::U16:
            assert(count == 1);
            return normalized ? DXGI_FORMAT_R8G8_UNORM : DXGI_FORMAT_R8G8_UINT;
        case gtype::S16:
            assert(count == 1);
            return normalized ? DXGI_FORMAT_R8G8_SNORM : DXGI_FORMAT_R8G8_SINT;
        case gtype::U32:
            assert(count <= 4);
            if (count == 1) return normalized ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UINT;
            if (count == 2) return normalized ? DXGI_FORMAT_R16G16B16A16_UNORM : DXGI_FORMAT_R16G16B16A16_UINT;
            if (count == 3) {
                assert(!normalized);
                return DXGI_FORMAT_R32G32B32_UINT;
            }
            if (count == 4) {
                assert(!normalized);
                return DXGI_FORMAT_R32G32B32A32_UINT;
            }
        case gtype::S32:
            assert(count <= 4);
            if (count == 1) return normalized ? DXGI_FORMAT_R8G8B8A8_SNORM : DXGI_FORMAT_R8G8B8A8_SINT;
            if (count == 2) return normalized ? DXGI_FORMAT_R16G16B16A16_SNORM : DXGI_FORMAT_R16G16B16A16_SINT;
            if (count == 3) {
                assert(!normalized);
                return DXGI_FORMAT_R32G32B32_SINT;
            }
            if (count == 4) {
                assert(!normalized);
                return DXGI_FORMAT_R32G32B32A32_SINT;
            }
        case gtype::F32:
            assert(count <= 4);
            if (count == 1) return DXGI_FORMAT_R32_FLOAT;
            if (count == 2) return DXGI_FORMAT_R32G32_FLOAT;
            if (count == 3) return DXGI_FORMAT_R32G32B32_FLOAT;
            if (count == 4) return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default:
            assert(false);
            return DXGI_FORMAT_UNKNOWN;
    }
}

void d3d_buffer_set_input_layout(buffer *b, const buffer_layout &layout) {
    assert(b->Graphics->CurrentlyBoundShader);
    assert(b->Graphics->CurrentlyBoundShader->D3D.VSBlob);

    b->Stride = layout.TotalSize;

    SAFE_RELEASE(b->D3D.Layout);

    auto *desc = allocate_array(D3D11_INPUT_ELEMENT_DESC, layout.Elements.Count, Context.TemporaryAlloc);
    auto *p = desc;
    For(layout.Elements) {
        const char *name = it.Name.to_c_string(Context.TemporaryAlloc);
        *p++ = {name,
                0,
                gtype_and_count_to_dxgi_format(it.Type, it.Count, it.Normalized),
                0,
                it.AlignedByteOffset,
                D3D11_INPUT_PER_VERTEX_DATA,
                0};
    }

    auto *vs = (ID3DBlob *) b->Graphics->CurrentlyBoundShader->D3D.VSBlob;
    DX_CHECK(b->Graphics->D3D.Device->CreateInputLayout(desc, (u32) layout.Elements.Count, vs->GetBufferPointer(), vs->GetBufferSize(), &b->D3D.Layout));
}

void *d3d_buffer_map(buffer *b, buffer_map_access access) {
    D3D11_MAP d3dMap;
    if (access == buffer_map_access::Read) d3dMap = D3D11_MAP_READ;
    if (access == buffer_map_access::Read_Write) d3dMap = D3D11_MAP_READ_WRITE;
    if (access == buffer_map_access::Write) d3dMap = D3D11_MAP_WRITE;
    if (access == buffer_map_access::Write_Discard_Previous) d3dMap = D3D11_MAP_WRITE_DISCARD;
    if (access == buffer_map_access::Write_Unsynchronized) d3dMap = D3D11_MAP_WRITE_NO_OVERWRITE;

    DX_CHECK(b->Graphics->D3D.DeviceContext->Map(b->D3D.Buffer, 0, d3dMap, 0,
                                                 (D3D11_MAPPED_SUBRESOURCE *) &b->D3D.MappedData));
    return ((D3D11_MAPPED_SUBRESOURCE *) (&b->D3D.MappedData))->pData;
}

void d3d_buffer_unmap(buffer *b) { b->Graphics->D3D.DeviceContext->Unmap(b->D3D.Buffer, 0); }

void d3d_buffer_bind(buffer *b, primitive_topology topology, u32 offset, u32 stride, shader_type shaderType,
                     u32 position) {
    if (b->Type == buffer_type::Vertex_Buffer) {
        if (stride == 0) stride = (u32) b->Stride;

        D3D_PRIMITIVE_TOPOLOGY d3dTopology = (D3D_PRIMITIVE_TOPOLOGY) 0;
        if (topology == primitive_topology::LineList) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        if (topology == primitive_topology::LineStrip) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        if (topology == primitive_topology::PointList) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        if (topology == primitive_topology::TriangleList) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        if (topology == primitive_topology::TriangleStrip) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        assert((s32) d3dTopology);

        b->Graphics->D3D.DeviceContext->IASetPrimitiveTopology(d3dTopology);

        b->Graphics->D3D.DeviceContext->IASetInputLayout(b->D3D.Layout);
        b->Graphics->D3D.DeviceContext->IASetVertexBuffers(0, 1, &b->D3D.Buffer, &stride, &offset);
    } else if (b->Type == buffer_type::Index_Buffer) {
        b->Graphics->D3D.DeviceContext->IASetIndexBuffer(b->D3D.Buffer, DXGI_FORMAT_R32_UINT, offset);
    } else if (b->Type == buffer_type::Shader_Uniform_Buffer) {
        if (shaderType == shader_type::Vertex_Shader) {
            b->Graphics->D3D.DeviceContext->VSSetConstantBuffers(position, 1, &b->D3D.Buffer);
        } else if (shaderType == shader_type::Fragment_Shader) {
            b->Graphics->D3D.DeviceContext->PSSetConstantBuffers(position, 1, &b->D3D.Buffer);
        }
    } else {
        assert(false);
    }
}

void d3d_buffer_unbind(buffer *b) {
    ID3D11Buffer *buffer = null;
    if (b->Type == buffer_type::Vertex_Buffer) {
        u32 stride = 0, offset = 0;
        b->Graphics->D3D.DeviceContext->IASetInputLayout(null);
        b->Graphics->D3D.DeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
    } else if (b->Type == buffer_type::Index_Buffer) {
        b->Graphics->D3D.DeviceContext->IASetIndexBuffer(null, DXGI_FORMAT_R32_UINT, 0);
    } else if (b->Type == buffer_type::Shader_Uniform_Buffer) {
    } else {
        assert(false);
    }
}

void d3d_buffer_release(buffer *b) {
    SAFE_RELEASE(b->D3D.Buffer);
    SAFE_RELEASE(b->D3D.Layout);
}

buffer::impl g_D3DBufferImpl = {d3d_buffer_init, d3d_buffer_set_input_layout, d3d_buffer_map, d3d_buffer_unmap,
                                d3d_buffer_bind, d3d_buffer_unbind, d3d_buffer_release};

LSTD_END_NAMESPACE

#endif
