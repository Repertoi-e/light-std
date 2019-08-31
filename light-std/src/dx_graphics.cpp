#include "lstd/common.h"

#if OS == WINDOWS

#include "lstd/dx_graphics.h"
#include "lstd/os.h"

#undef MAC
#undef _MAC
#include <Windows.h>

#include <d3d10_1.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

LSTD_BEGIN_NAMESPACE

DXGI_FORMAT gtype_and_count_to_dxgi_format(gtype type, size_t count, bool normalized) {
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

void dx_shader::bind() {
    D3DGraphics->D3DBoundShader = this;

    D3DGraphics->D3DDeviceContext->VSSetShader(D3DData.VS, null, 0);
    D3DGraphics->D3DDeviceContext->PSSetShader(D3DData.PS, null, 0);
}

#if defined DEBUG || defined RELEASE
void dx_shader::unbind() {
    D3DGraphics->D3DBoundShader = null;

    D3DGraphics->D3DDeviceContext->VSSetShader(null, null, 0);
    D3DGraphics->D3DDeviceContext->PSSetShader(null, null, 0);
}
#endif

void dx_shader::release() {
    SAFE_RELEASE(D3DData.VS);
    SAFE_RELEASE(D3DData.PS);

    auto *vs = (ID3DBlob *) D3DData.VSBlob;
    auto *ps = (ID3DBlob *) D3DData.PSBlob;
    SAFE_RELEASE(vs);
    SAFE_RELEASE(ps);
    D3DData.VSBlob = null;
    D3DData.PSBlob = null;
}

void dx_texture_2D::bind(u32 slot) {
    D3DGraphics->D3DDeviceContext->PSSetShaderResources(slot, 1, &D3DResourceView);
    D3DGraphics->D3DDeviceContext->PSSetSamplers(slot, 1, &D3DSamplerState);
}

#if defined DEBUG || defined RELEASE
void dx_texture_2D::unbind(u32 slot) {
    ID3D11ShaderResourceView *rv = null;
    D3DGraphics->D3DDeviceContext->PSSetShaderResources(slot, 1, &rv);
}
#endif

void dx_texture_2D::set_data(const char *pixels) {
    D3D11_MAPPED_SUBRESOURCE mappedData;
    zero_memory(&mappedData, sizeof(mappedData));

    DXCHECK(D3DGraphics->D3DDeviceContext->Map(D3DTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    for (u32 i = 0; i < Width * Height * 4; i += 4) {
        ((u8 *) mappedData.pData)[i + 0] = 0xff;
        ((u8 *) mappedData.pData)[i + 1] = 0xff;
        ((u8 *) mappedData.pData)[i + 2] = 0xff;
        ((u8 *) mappedData.pData)[i + 3] = ((u8 *) pixels)[i / 2 + 1];
    }
    D3DGraphics->D3DDeviceContext->Unmap(D3DTexture, 0);
}

void dx_texture_2D::set_data(u32 color) { assert(false); }

void dx_texture_2D::release() {
    SAFE_RELEASE(D3DTexture);
    SAFE_RELEASE(D3DResourceView);
    SAFE_RELEASE(D3DSamplerState);
}

void dx_buffer::set_input_layout(buffer_layout *layout) {
    Layout = layout;

    SAFE_RELEASE(D3DLayout);

    auto *desc = new (Context.TemporaryAlloc) D3D11_INPUT_ELEMENT_DESC[layout->Elements.Count];
    auto *p = desc;
    For(layout->Elements) {
        *p++ = {it.Name.to_c_string(Context.TemporaryAlloc),
                0,
                gtype_and_count_to_dxgi_format(it.Type, it.Count, it.Normalized),
                0,
                it.AlignedByteOffset,
                D3D11_INPUT_PER_VERTEX_DATA,
                0};
    }

    auto *vs = (ID3DBlob *) D3DGraphics->D3DBoundShader->D3DData.VSBlob;
    DXCHECK(D3DGraphics->D3DDevice->CreateInputLayout(desc, (u32) layout->Elements.Count, vs->GetBufferPointer(),
                                                      vs->GetBufferSize(), &D3DLayout));
}

void *dx_buffer::map(map_access access) {
    D3D11_MAP d3dMap;
    if (access == map_access::READ) d3dMap = D3D11_MAP_READ;
    if (access == map_access::READ_WRITE) d3dMap = D3D11_MAP_READ_WRITE;
    if (access == map_access::WRITE) d3dMap = D3D11_MAP_WRITE;
    if (access == map_access::WRITE_DISCARD_PREVIOUS) d3dMap = D3D11_MAP_WRITE_DISCARD;
    if (access == map_access::WRITE_UNSYNCHRONIZED) d3dMap = D3D11_MAP_WRITE_NO_OVERWRITE;

    DXCHECK(D3DGraphics->D3DDeviceContext->Map(D3DBuffer, 0, d3dMap, 0, (D3D11_MAPPED_SUBRESOURCE *) &MappedData));
    return ((D3D11_MAPPED_SUBRESOURCE *) (&MappedData))->pData;
}

void dx_buffer::unmap() { D3DGraphics->D3DDeviceContext->Unmap(D3DBuffer, 0); }

void dx_buffer::bind(bind_data bindData) {
    if (Type == type::VERTEX_BUFFER) {
        if (bindData.Stride == 0) bindData.Stride = (u32) Layout->TotalSize;

        D3D_PRIMITIVE_TOPOLOGY d3dTopology = (D3D_PRIMITIVE_TOPOLOGY) 0;
        if (bindData.Topology == primitive_topology::LineList) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        if (bindData.Topology == primitive_topology::LineStrip) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        if (bindData.Topology == primitive_topology::PointList) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        if (bindData.Topology == primitive_topology::TriangleList) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        if (bindData.Topology == primitive_topology::TriangleStrip)
            d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        assert((s32) d3dTopology);

        D3DGraphics->D3DDeviceContext->IASetPrimitiveTopology(d3dTopology);

        D3DGraphics->D3DDeviceContext->IASetInputLayout(D3DLayout);
        D3DGraphics->D3DDeviceContext->IASetVertexBuffers(0, 1, &D3DBuffer, &bindData.Stride, &bindData.Offset);
    } else if (Type == type::INDEX_BUFFER) {
        D3DGraphics->D3DDeviceContext->IASetIndexBuffer(D3DBuffer, DXGI_FORMAT_R32_UINT, bindData.Offset);
    } else if (Type == type::SHADER_UNIFORM_BUFFER) {
        if (bindData.ShaderType == shader::type::VERTEX_SHADER) {
            D3DGraphics->D3DDeviceContext->VSSetConstantBuffers(bindData.Position, 1, &D3DBuffer);

        } else if (bindData.ShaderType == shader::type::FRAGMENT_SHADER) {
            D3DGraphics->D3DDeviceContext->PSSetConstantBuffers(bindData.Position, 1, &D3DBuffer);
        }
    } else {
        assert(false);
    }
}

#if defined DEBUG || defined RELEASE
void dx_buffer::unbind() {
    if (Type == type::VERTEX_BUFFER) {
    } else if (Type == type::INDEX_BUFFER) {
    } else if (Type == type::SHADER_UNIFORM_BUFFER) {
    } else {
        assert(false);
    }
}
#endif

void dx_buffer::release() {
    SAFE_RELEASE(D3DBuffer);
    SAFE_RELEASE(D3DLayout);
}

void dx_graphics::init() {
    IDXGIFactory *factory;
    DXCHECK(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **) &factory));
    defer(SAFE_RELEASE(factory));

    IDXGIAdapter *adapter;
    DXCHECK(factory->EnumAdapters(0, &adapter));
    defer(SAFE_RELEASE(adapter));

    IDXGIOutput *adapterOutput;
    defer(SAFE_RELEASE(adapterOutput));
    DXCHECK(adapter->EnumOutputs(0, &adapterOutput));

    u32 numModes = 0;
    DXCHECK(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, null));
    assert(numModes);

    DXGI_MODE_DESC *displayModeList = new (Context.TemporaryAlloc) DXGI_MODE_DESC[numModes];
    DXCHECK(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes,
                                              displayModeList));

    DXGI_ADAPTER_DESC adapterDesc;
    DXCHECK(adapter->GetDesc(&adapterDesc));

    auto adapterStr = string(c_string_strlen(adapterDesc.Description));
    utf16_to_utf8(adapterDesc.Description, const_cast<char *>(adapterStr.Data), &adapterStr.ByteLength);
    adapterStr.Length = utf8_strlen(adapterStr.Data, adapterStr.ByteLength);

    fmt::print("{!YELLOW}----------------------------------\n");
    fmt::print(" Direct3D 11:\n");
    fmt::print("    {}\n", adapterStr);
    fmt::print("    VRAM: {} MB\n", adapterDesc.DedicatedVideoMemory / 1024 / 1024);
    fmt::print("----------------------------------\n\n{!}");

    // In order to support Direct2D, add D3D11_CREATE_DEVICE_BGRA_SUPPORT to _creationFlags_
    u32 creationFlags = 0;
#if defined DEBUG || defined RELEASE
    // If the project is not in Dist configuration, enable the debug layer.
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D11CreateDevice(null, D3D_DRIVER_TYPE_HARDWARE, null, creationFlags, null, 0, D3D11_SDK_VERSION, &D3DDevice, null,
                      &D3DDeviceContext);

    // @TODO: D3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampleCount, &sampleQuality);

    D3D11_BLEND_DESC blendDest;
    zero_memory(&blendDest, sizeof(blendDest));
    {
        blendDest.AlphaToCoverageEnable = false;
        blendDest.IndependentBlendEnable = false;

        blendDest.RenderTarget[0].BlendEnable = true;
        blendDest.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDest.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDest.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDest.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blendDest.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDest.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDest.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }
    DXCHECK(D3DDevice->CreateBlendState(&blendDest, &D3DBlendStates[0]));

    zero_memory(&blendDest, sizeof(blendDest));
    {
        blendDest.RenderTarget[0].BlendEnable = false;
        blendDest.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }
    DXCHECK(D3DDevice->CreateBlendState(&blendDest, &D3DBlendStates[1]));

    D3D11_DEPTH_STENCIL_DESC stencilDesc;
    zero_memory(&stencilDesc, sizeof(stencilDesc));
    {
        stencilDesc.DepthEnable = true;
        stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        stencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

        stencilDesc.StencilEnable = true;
        stencilDesc.StencilReadMask = 0xff;
        stencilDesc.StencilWriteMask = 0xff;

        stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        stencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        stencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        stencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        stencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    }
    DXCHECK(D3DDevice->CreateDepthStencilState(&stencilDesc, &D3DDepthStencilStates[0]));

    zero_memory(&stencilDesc, sizeof(stencilDesc));
    {
        stencilDesc.DepthEnable = false;
        stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        stencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

        stencilDesc.StencilEnable = false;

        stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        stencilDesc.BackFace = stencilDesc.FrontFace;
    }
    DXCHECK(D3DDevice->CreateDepthStencilState(&stencilDesc, &D3DDepthStencilStates[1]));

    TargetWindows.append();  // Add a null target
}

void dx_graphics::add_target_window(window *win) {
    assert(win);

    auto *targetWindow = TargetWindows.append();
    targetWindow->Window = win;

    vec2i windowSize = win->get_size();

    DXGI_SWAP_CHAIN_DESC desc;
    zero_memory(&desc, sizeof(desc));
    {
        desc.BufferCount = 1;
        desc.BufferDesc.Width = windowSize.x;
        desc.BufferDesc.Height = windowSize.y;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferDesc.RefreshRate.Numerator =
            win->Flags & window::VSYNC ? os_monitor_from_window(win)->CurrentMode.RefreshRate : 0;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = win->PlatformData.Win32.hWnd;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        desc.SampleDesc.Count = 1;
        desc.Windowed = !win->is_fullscreen();
    }

    IDXGIDevice *device;
    DXCHECK(D3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void **) &device));
    defer(SAFE_RELEASE(device));

    IDXGIAdapter *adapter;
    device->GetAdapter(&adapter);
    defer(SAFE_RELEASE(adapter));

    IDXGIFactory *factory = null;
    adapter->GetParent(__uuidof(IDXGIFactory), (void **) &factory);
    defer(SAFE_RELEASE(factory));

    DXCHECK(factory->CreateSwapChain(D3DDevice, &desc, &targetWindow->D3DSwapChain));

    targetWindow->ResizeCallbackID =
        win->WindowFramebufferResizedEvent.connect({this, &dx_graphics::window_changed_size});
    window_changed_size({win, windowSize.x, windowSize.y});
}

void dx_graphics::remove_target_window(window *win) {
    target_window *targetWindow = null;
    For(range(TargetWindows.Count)) {
        targetWindow = &TargetWindows[it];
        if (targetWindow->Window == win) break;
    }
    assert(targetWindow);

    if (CurrentTargetWindow == targetWindow) set_current_target_window(null);

    targetWindow->Window->WindowFramebufferResizedEvent.disconnect(targetWindow->ResizeCallbackID);

    targetWindow->D3DSwapChain->SetFullscreenState(false, null);
    SAFE_RELEASE(targetWindow->D3DSwapChain);
    SAFE_RELEASE(targetWindow->D3DBackBuffer);
    SAFE_RELEASE(targetWindow->D3DDepthStencilBuffer);
    SAFE_RELEASE(targetWindow->D3DDepthStencilView);
    SAFE_RELEASE(targetWindow->D3DRasterState[0]);
    SAFE_RELEASE(targetWindow->D3DRasterState[1]);
    SAFE_RELEASE(targetWindow->D3DRasterState[2]);

    TargetWindows.remove(targetWindow - TargetWindows.Data);
}

void dx_graphics::set_current_target_window(window *win) {
    target_window *targetWindow = null;
    For(range(TargetWindows.Count)) {
        targetWindow = &TargetWindows[it];
        if (targetWindow->Window == win) break;
    }
    assert(targetWindow);

    CurrentTargetWindow = targetWindow;

    D3DDeviceContext->OMSetRenderTargets(1, &targetWindow->D3DBackBuffer, targetWindow->D3DDepthStencilView);

    if (win) {
        set_cull_mode(targetWindow->CullMode);

        vec2i windowSize = win->get_size();
        D3D11_VIEWPORT viewport;
        zero_memory(&viewport, sizeof(viewport));
        {
            viewport.Width = (f32) windowSize.x;
            viewport.Height = (f32) windowSize.y;
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            viewport.TopLeftX = 0.0f;
            viewport.TopLeftY = 0.0f;
        }
        D3DDeviceContext->RSSetViewports(1, &viewport);
    }
}

void dx_graphics::clear_color(vec4 color) {
    assert(CurrentTargetWindow->Window);  // May be a null target

    if (!CurrentTargetWindow->Window->is_visible()) return;

    f32 c[] = {color.r, color.g, color.b, color.a};

    D3DDeviceContext->ClearRenderTargetView(CurrentTargetWindow->D3DBackBuffer, c);
    D3DDeviceContext->ClearDepthStencilView(CurrentTargetWindow->D3DDepthStencilView,
                                            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void dx_graphics::set_blend(bool enabled) {
    D3DDeviceContext->OMSetBlendState(enabled ? D3DBlendStates[0] : D3DBlendStates[1], null, 0xffffffff);
}

void dx_graphics::set_depth_testing(bool enabled) {
    D3DDeviceContext->OMSetDepthStencilState(enabled ? D3DDepthStencilStates[0] : D3DDepthStencilStates[1], 0);
}

void dx_graphics::set_cull_mode(cull mode) {
    assert(CurrentTargetWindow->Window);
    D3DDeviceContext->RSSetState(CurrentTargetWindow->D3DRasterState[(size_t) mode]);
    CurrentTargetWindow->CullMode = mode;
}

void dx_graphics::create_buffer(buffer *buffer, buffer::type type, buffer::usage usage, size_t size) {
    auto *dxBuffer = (dx_buffer *) buffer;

    dxBuffer->D3DGraphics = this;
    dxBuffer->Type = type;
    dxBuffer->Usage = usage;
    dxBuffer->Size = size;

    assert(usage != buffer::IMMUTABLE && "Immutable buffers must be created with initial data");

    D3D11_BUFFER_DESC desc;
    zero_memory(&desc, sizeof(desc));
    {
        assert(size <= numeric_info<u32>::max());
        desc.ByteWidth = (u32) size;
        if (usage == buffer::IMMUTABLE) desc.Usage = D3D11_USAGE_IMMUTABLE;
        if (usage == buffer::DYNAMIC) desc.Usage = D3D11_USAGE_DYNAMIC;
        if (usage == buffer::STAGING) desc.Usage = D3D11_USAGE_STAGING;

        if (type == buffer::VERTEX_BUFFER) desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        if (type == buffer::INDEX_BUFFER) desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        if (type == buffer::SHADER_UNIFORM_BUFFER) desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        if (usage == buffer::DYNAMIC) desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (usage == buffer::STAGING) desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    }
    DXCHECK(D3DDevice->CreateBuffer(&desc, null, &dxBuffer->D3DBuffer));
}

void dx_graphics::create_buffer(buffer *buffer, buffer::type type, buffer::usage usage, const char *initialData,
                                size_t initialDataSize) {
    auto *dxBuffer = (dx_buffer *) buffer;

    dxBuffer->D3DGraphics = this;
    dxBuffer->Type = type;
    dxBuffer->Usage = usage;
    dxBuffer->Size = initialDataSize;

    D3D11_BUFFER_DESC desc;
    zero_memory(&desc, sizeof(desc));
    {
        assert(initialDataSize <= numeric_info<u32>::max());
        desc.ByteWidth = (u32) initialDataSize;
        if (usage == buffer::DEFAULT) desc.Usage = D3D11_USAGE_DEFAULT;
        if (usage == buffer::IMMUTABLE) desc.Usage = D3D11_USAGE_IMMUTABLE;
        if (usage == buffer::DYNAMIC) desc.Usage = D3D11_USAGE_DYNAMIC;
        if (usage == buffer::STAGING) desc.Usage = D3D11_USAGE_STAGING;

        if (type == buffer::VERTEX_BUFFER) desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        if (type == buffer::INDEX_BUFFER) desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        if (type == buffer::SHADER_UNIFORM_BUFFER) desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        if (usage == buffer::DEFAULT) desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        if (usage == buffer::DYNAMIC) desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (usage == buffer::STAGING) desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    }

    assert(initialData);

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = initialData;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;
    DXCHECK(D3DDevice->CreateBuffer(&desc, &data, &dxBuffer->D3DBuffer));
}

void dx_graphics::create_texture_2D(texture_2D *texture, string name, u32 width, u32 height, texture::filter filter,
                                    texture::wrap wrap) {
    auto *dxTexture = (dx_texture_2D *) texture;
    dxTexture->D3DGraphics = this;

    clone(&dxTexture->Name, name);

    dxTexture->Wrap = wrap;
    dxTexture->Filter = filter;

    dxTexture->Width = width;
    dxTexture->Height = height;

    D3D11_TEXTURE2D_DESC textureDesc;
    zero_memory(&textureDesc, sizeof(textureDesc));
    {
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Usage = D3D11_USAGE_DYNAMIC;
        textureDesc.CPUAccessFlags = textureDesc.Usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
    }
    DXCHECK(D3DDevice->CreateTexture2D(&textureDesc, null, &dxTexture->D3DTexture));

    D3D11_SHADER_RESOURCE_VIEW_DESC rvDesc;
    zero_memory(&rvDesc, sizeof(rvDesc));
    {
        rvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        rvDesc.Texture2D.MipLevels = 1;
    }
    DXCHECK(D3DDevice->CreateShaderResourceView(dxTexture->D3DTexture, &rvDesc, &dxTexture->D3DResourceView));

    D3D11_SAMPLER_DESC samplerDesc;
    zero_memory(&samplerDesc, sizeof(samplerDesc));
    {
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = 11;
        samplerDesc.Filter =
            filter == texture::LINEAR ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    }
    DXCHECK(D3DDevice->CreateSamplerState(&samplerDesc, &dxTexture->D3DSamplerState));
}

void dx_graphics::create_texture_2D_from_file(texture_2D *texture, string name, file::path path, bool flipX, bool flipY,
                                              texture::filter filter, texture::wrap wrap) {
    auto *dxTexture = (dx_texture_2D *) texture;
    dxTexture->D3DGraphics = this;

    clone(&dxTexture->Name, name);
    clone(&dxTexture->FilePath, path);

    // We don't load images from disc, yet.
    assert(false);
}

static ID3DBlob *compile(string source, const char *profile, const char *main) {
    ID3DBlob *shaderBlob = null, *errorBlob = null;
    DXCHECK(D3DCompile(source.Data, source.ByteLength, null, null, null, main, profile, D3DCOMPILE_DEBUG, 0,
                       &shaderBlob, &errorBlob));
    if (errorBlob) {
        fmt::print("... shader compile errors (profile = {!GRAY}{}{!}):\n{!YELLOW}{}\n{!}\n", profile,
                   (const char *) errorBlob->GetBufferPointer());
        assert(false);
        errorBlob->Release();
    }
    return shaderBlob;
}

static gtype string_to_gtype(string type) {
    size_t digit = type.find_any_of("0123456789");
    if (digit != npos) {
        size_t x = type.find('x');
        string scalarType = type.substring(0, digit);
        if (scalarType == "bool")
            return (gtype)((u32) gtype::BOOL_1x1 + (type[digit] - '0') * 4 + (x == npos ? 0 : type[x] - '0'));
        if (scalarType == "int" || scalarType == "int32")
            return (gtype)((u32) gtype::S32_1x1 + (type[digit] - '0') * 4 + (x == npos ? 0 : type[x] - '0'));
        if (scalarType == "uint" || scalarType == "uint32" || scalarType == "dword")
            return (gtype)((u32) gtype::U32_1x1 + (type[digit] - '0') * 4 + (x == npos ? 0 : type[x] - '0'));
        if (scalarType == "float")
            return (gtype)((u32) gtype::F32_1x1 + (type[digit] - '0') * 4 + (x == npos ? 0 : type[x] - '0'));
    } else {
        if (type == "bool") return gtype::BOOL;
        if (type == "int" || type == "int32") return gtype::S32;
        if (type == "uint" || type == "uint32" || type == "dword") return gtype::U32;
        if (type == "float") return gtype::F32;
    }
    return gtype::Unknown;
}

void dx_graphics::create_shader(shader *shader, string name, file::path path) {
    auto *dxShader = (dx_shader *) shader;
    dxShader->D3DGraphics = this;

    clone(&dxShader->Name, name);
    clone(&dxShader->FilePath, path);

    auto handle = file::handle(path);

    string source;
    if (!handle.read_entire_file(&source)) return;

    dxShader->D3DData.VSBlob = compile(source, "vs_4_0", "VSMain");
    dxShader->D3DData.PSBlob = compile(source, "ps_4_0", "PSMain");

    auto *vs = (ID3DBlob *) dxShader->D3DData.VSBlob;
    auto *ps = (ID3DBlob *) dxShader->D3DData.PSBlob;

    D3DDevice->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), null, &dxShader->D3DData.VS);
    D3DDevice->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), null, &dxShader->D3DData.PS);

    // Remove comments
    size_t startPos;
    while ((startPos = source.find("/*")) != npos) {
        size_t endPos = source.find("*/");
        assert(endPos != npos);
        source.remove(startPos, endPos);
    }

    while ((startPos = source.find("//")) != npos) {
        size_t endPos = source.find('\n');
        assert(endPos != npos);
        source.remove(startPos, endPos);
    }

    // @TODO Parse shaders structs

    size_t cbuffer;
    // Parse constant buffers and store the metadata
    while ((cbuffer = source.find("cbuffer")) != npos) {
        size_t closingBraces = 0;
        size_t brace = 0;
        while (true) {
            brace = source.find('}');
            if (source.substring(cbuffer, brace).count('{') == closingBraces) break;
        }

        string block = source.substring(cbuffer, brace);

        // Tokenize
        array<string> tokens;
        {
            size_t start = 0;
            size_t end = block.find_any_of(" \t\n");

            while (end <= npos) {
                string token = block.substring(start, (end == npos ? block.Length : end));
                if (token) tokens.append(token);
                if (end == npos) break;
                start = end + 1;
                end = block.find_any_of(" \t\n", start);
            }
        }

        size_t tokenIndex = 1;

        shader::uniform_buffer uniformBuffer;
        clone(&uniformBuffer.Name, tokens[tokenIndex++]);

        if (tokens[tokenIndex++] != ":") {
            fmt::print("... error when parsing shader, no register found in constant buffer declaration!\n");
            fmt::print("    Here is the block:\n{!YELLOW}{}{!}\n", block);
            assert(false);
        }

        string reg = tokens[tokenIndex++];
        auto it = reg.begin();
        while (!is_digit(*it)) ++it;
        while (is_digit(*it)) {
            uniformBuffer.Position *= 10;
            uniformBuffer.Position += *it - '0';
            ++it;
        }

        tokenIndex++;  // "{"
        while (tokens[tokenIndex] != "}") {
            string type = tokens[tokenIndex++];
            if (type == "linear" || type == "centroid" || type == "nointerpolation" || type == "noperspective" ||
                type == "sample") {
                type = tokens[tokenIndex++];
            }

            // @TODO !!!
            if (type == "struct") {
                assert(false);
            }

            string name = tokens[tokenIndex++];
            if (name[-1] == ';') {
                name = name.substring(0, -1);
            } else {
                assert(tokens[tokenIndex++] == ";");
            }

            if (uniformBuffer.ByteSize % 16 != 0) {
                uniformBuffer.ByteSize = ((uniformBuffer.ByteSize >> 4) + 1) << 4;
            }

            shader::uniform decl;
            decl.Name = name;
            decl.Type = string_to_gtype(type);
            decl.Offset = uniformBuffer.ByteSize;
            decl.ByteSize = get_size_of_base_gtype_in_bits(decl.Type) / 8;  // Guaranteed not to be 1-bit
            decl.Count = get_count_of_gtype(decl.Type);
            uniformBuffer.ByteSize += decl.ByteSize * decl.Count;
            uniformBuffer.Uniforms.append(decl);
        }
        move(&dxShader->UniformBuffers.append(uniformBuffer)->Uniforms, &uniformBuffer.Uniforms);
    }
}

void dx_graphics::draw(size_t vertices) { D3DDeviceContext->Draw((u32) vertices, 0); }

void dx_graphics::draw_indexed(size_t indices) { D3DDeviceContext->DrawIndexed((u32) indices, 0, 0); }

void dx_graphics::swap() {
    assert(CurrentTargetWindow->Window);  // May be a null target

    if (!CurrentTargetWindow->Window->is_visible()) return;
    CurrentTargetWindow->D3DSwapChain->Present(CurrentTargetWindow->Window->Flags & window::VSYNC ? 1 : 0, 0);
}

void dx_graphics::release() {
    For(range(TargetWindows.Count)) {
        target_window *win = &TargetWindows[it];
        if (win->Window) {
            win->Window->WindowFramebufferResizedEvent.disconnect(win->ResizeCallbackID);

            win->D3DSwapChain->SetFullscreenState(false, null);
            SAFE_RELEASE(win->D3DSwapChain);
            SAFE_RELEASE(win->D3DBackBuffer);
            SAFE_RELEASE(win->D3DDepthStencilBuffer);
            SAFE_RELEASE(win->D3DDepthStencilView);
            SAFE_RELEASE(win->D3DRasterState[0]);
            SAFE_RELEASE(win->D3DRasterState[1]);
            SAFE_RELEASE(win->D3DRasterState[2]);
        }
    }

    SAFE_RELEASE(D3DDevice);
    SAFE_RELEASE(D3DDeviceContext);

    SAFE_RELEASE(D3DBlendStates[0]);
    SAFE_RELEASE(D3DBlendStates[1]);
    SAFE_RELEASE(D3DDepthStencilStates[0]);
    SAFE_RELEASE(D3DDepthStencilStates[1]);
}

void dx_graphics::window_changed_size(const window_framebuffer_resized_event &e) {
    // Find the target window that corresponds to the window that was resized
    target_window *targetWindow = null;
    For(range(TargetWindows.Count)) {
        targetWindow = &TargetWindows[it];
        if (targetWindow->Window == e.Window) break;
    }
    assert(targetWindow);

    if (!e.Window->is_visible()) return;

    SAFE_RELEASE(targetWindow->D3DBackBuffer);
    SAFE_RELEASE(targetWindow->D3DDepthStencilView);
    SAFE_RELEASE(targetWindow->D3DDepthStencilBuffer);
    SAFE_RELEASE(targetWindow->D3DRasterState[0]);
    SAFE_RELEASE(targetWindow->D3DRasterState[1]);
    SAFE_RELEASE(targetWindow->D3DRasterState[2]);

    set_current_target_window(null);
    D3DDeviceContext->Flush();

    ID3D11Debug *debug;
    D3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void **) &debug);
    debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);

    DXCHECK(targetWindow->D3DSwapChain->ResizeBuffers(1, e.Width, e.Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    ID3D11Texture2D *swapChainBackBuffer;
    DXCHECK(targetWindow->D3DSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &swapChainBackBuffer));
    DXCHECK(D3DDevice->CreateRenderTargetView(swapChainBackBuffer, null, &targetWindow->D3DBackBuffer));
    SAFE_RELEASE(swapChainBackBuffer);

    D3D11_TEXTURE2D_DESC textureDesc;
    zero_memory(&textureDesc, sizeof(textureDesc));
    {
        textureDesc.Width = e.Width;
        textureDesc.Height = e.Height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
    }

    DXCHECK(D3DDevice->CreateTexture2D(&textureDesc, null, &targetWindow->D3DDepthStencilBuffer));
    DXCHECK(D3DDevice->CreateDepthStencilView(targetWindow->D3DDepthStencilBuffer, null,
                                              &targetWindow->D3DDepthStencilView));

    D3D11_RASTERIZER_DESC rDesc;
    zero_memory(&rDesc, sizeof(rDesc));
    {
        rDesc.FillMode = D3D11_FILL_SOLID;
        rDesc.CullMode = D3D11_CULL_NONE;
        rDesc.ScissorEnable = true;
        rDesc.DepthClipEnable = true;
    }
    DXCHECK(D3DDevice->CreateRasterizerState(&rDesc, &targetWindow->D3DRasterState[(size_t) cull::None]));

    zero_memory(&rDesc, sizeof(rDesc));
    {
        rDesc.FillMode = D3D11_FILL_SOLID;
        rDesc.CullMode = D3D11_CULL_FRONT;
        rDesc.ScissorEnable = false;
        rDesc.DepthClipEnable = true;
    }
    DXCHECK(D3DDevice->CreateRasterizerState(&rDesc, &targetWindow->D3DRasterState[(size_t) cull::Front]));

    zero_memory(&rDesc, sizeof(rDesc));
    {
        rDesc.FillMode = D3D11_FILL_SOLID;
        rDesc.CullMode = D3D11_CULL_BACK;
        rDesc.ScissorEnable = false;
        rDesc.DepthClipEnable = true;
    }
    DXCHECK(D3DDevice->CreateRasterizerState(&rDesc, &targetWindow->D3DRasterState[(size_t) cull::Back]));
}

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
