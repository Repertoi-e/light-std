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

LSTD_BEGIN_NAMESPACE

namespace g {
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

void dx_buffer::set_input_layout(buffer_layout *layout) {
    Layout = layout;

    SAFE_RELEASE(D3DLayout);

    auto *desc = new D3D11_INPUT_ELEMENT_DESC[layout->Elements.Count];
    defer(delete[] desc);

    auto *p = desc;
    For(layout->Elements) {
        *p++ = {it.Name.to_c_string(),
                0,
                gtype_and_count_to_dxgi_format(it.Type, it.Count, it.Normalized),
                0,
                it.AlignedByteOffset,
                D3D11_INPUT_PER_VERTEX_DATA,
                0};  // @Leak
    }

    auto *vs = (ID3DBlob *) D3DGraphics->D3DBoundShader->D3DData.VSBlob;
    DXCHECK(D3DGraphics->D3DDevice->CreateInputLayout(desc, (u32) layout->Elements.Count, vs->GetBufferPointer(),
                                                      vs->GetBufferSize(), &D3DLayout));
}

void *dx_buffer::map(map_access access) {
    D3D11_MAP d3dMap;
    if (access = map_access::READ) d3dMap = D3D11_MAP_READ;
    if (access = map_access::READ_WRITE) d3dMap = D3D11_MAP_READ_WRITE;
    if (access = map_access::WRITE) d3dMap = D3D11_MAP_WRITE;
    if (access = map_access::WRITE_DISCARD_PREVIOUS) d3dMap = D3D11_MAP_WRITE_DISCARD;
    if (access = map_access::WRITE_UNSYNCHRONIZED) d3dMap = D3D11_MAP_WRITE_NO_OVERWRITE;

    DXCHECK(D3DGraphics->D3DDeviceContext->Map(D3DBuffer, 0, d3dMap, 0, (D3D11_MAPPED_SUBRESOURCE *) &MappedData));
    return ((D3D11_MAPPED_SUBRESOURCE *) (&MappedData))->pData;
}

void dx_buffer::unmap() { D3DGraphics->D3DDeviceContext->Unmap(D3DBuffer, 0); }

void dx_buffer::bind(bind_data bindData) {
    if (Type == type::VERTEX_BUFFER) {
        if (bindData.Stride == 0) bindData.Stride = (u32) Layout->TotalSize;

        D3D_PRIMITIVE_TOPOLOGY d3dTopology;
        if (bindData.Topology == primitive_topology::LineList) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        if (bindData.Topology == primitive_topology::LineStrip) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        if (bindData.Topology == primitive_topology::PointList) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        if (bindData.Topology == primitive_topology::TriangleList) d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        if (bindData.Topology == primitive_topology::TriangleStrip)
            d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
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

void dx_graphics::init(window::window *targetWindow) {
    TargetWindow = targetWindow;

    IDXGIFactory *factory;
    DXCHECK(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **) &factory));

    IDXGIAdapter *adapter;

    DXCHECK(factory->EnumAdapters(0, &adapter));

    IDXGIOutput *adapterOutput;
    defer(SAFE_RELEASE(adapterOutput));
    DXCHECK(adapter->EnumOutputs(0, &adapterOutput));

    u32 numModes;
    DXCHECK(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, null));

    DXGI_MODE_DESC *displayModeList = new DXGI_MODE_DESC[numModes];
    defer(delete displayModeList);
    DXCHECK(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes,
                                              displayModeList));

    u32 numerator = 0, denominator = 0;
    for (u32 i = 0; i < numModes; i++) {
        if (displayModeList[i].Width == targetWindow->Width) {
            if (displayModeList[i].Height == targetWindow->Height) {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Denominator;
            }
        }
    }
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

    {
        DXGI_SWAP_CHAIN_DESC desc;
        zero_memory(&desc, sizeof(desc));
        {
            desc.BufferCount = 1;
            desc.BufferDesc.Width = targetWindow->Width;
            desc.BufferDesc.Height = targetWindow->Height;
            desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.BufferDesc.RefreshRate.Numerator = targetWindow->VSyncEnabled ? numerator : 0;
            desc.BufferDesc.RefreshRate.Denominator = targetWindow->VSyncEnabled ? denominator : 1;
            desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            desc.OutputWindow = *((HWND *) &targetWindow->PlatformData);
            desc.SampleDesc.Count = 1;
            desc.Windowed = true;
            desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        }

        SAFE_RELEASE(factory);
        SAFE_RELEASE(adapter);

        IDXGIDevice *dxgiDevice;
        DXCHECK(D3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void **) &dxgiDevice));

        dxgiDevice->GetAdapter(&adapter);
        adapter->GetParent(__uuidof(IDXGIFactory), (void **) &factory);

        DXCHECK(factory->CreateSwapChain(D3DDevice, &desc, &D3DSwapChain));
    }

    {
        D3D11_BLEND_DESC desc;
        zero_memory(&desc, sizeof(desc));
        {
            desc.AlphaToCoverageEnable = false;
            desc.IndependentBlendEnable = false;

            desc.RenderTarget[0].BlendEnable = true;
            desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        }
        DXCHECK(D3DDevice->CreateBlendState(&desc, &D3DBlendStates[0]));

        zero_memory(&desc, sizeof(desc));
        {
            desc.RenderTarget[0].BlendEnable = false;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        }
        DXCHECK(D3DDevice->CreateBlendState(&desc, &D3DBlendStates[1]));
    }

    {
        D3D11_DEPTH_STENCIL_DESC desc;
        zero_memory(&desc, sizeof(desc));
        {
            desc.DepthEnable = true;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D11_COMPARISON_LESS;

            desc.StencilEnable = true;
            desc.StencilReadMask = 0xff;
            desc.StencilWriteMask = 0xff;

            desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
            desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

            desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
            desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        }
        DXCHECK(D3DDevice->CreateDepthStencilState(&desc, &D3DDepthStencilStates[0]));

        zero_memory(&desc, sizeof(desc));
        {
            desc.DepthEnable = false;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            desc.DepthFunc = D3D11_COMPARISON_ALWAYS;

            desc.StencilEnable = true;
            desc.StencilReadMask = 0xff;
            desc.StencilWriteMask = 0xff;

            desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT;
            desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

            desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            desc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
        }
        DXCHECK(D3DDevice->CreateDepthStencilState(&desc, &D3DDepthStencilStates[1]));
    }

    change_size({targetWindow, targetWindow->Width, targetWindow->Height});

    set_blend(false);
    set_depth_testing(false);
}

void dx_graphics::clear_color(vec4 color) {
    f32 c[] = {color.r, color.g, color.b, color.a};

    D3DDeviceContext->ClearRenderTargetView(D3DBackBuffer, c);
    D3DDeviceContext->ClearDepthStencilView(D3DDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void dx_graphics::set_blend(bool enabled) {
    D3DDeviceContext->OMSetBlendState(enabled ? D3DBlendStates[0] : D3DBlendStates[1], null, 0xffffffff);
}

void dx_graphics::set_depth_testing(bool enabled) {
    D3DDeviceContext->OMSetDepthStencilState(enabled ? D3DDepthStencilStates[0] : D3DDepthStencilStates[1], 0);
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

void dx_graphics::create_shader(shader *shader, file::path path) {
    auto *dxShader = (dx_shader *) shader;
    dxShader->D3DGraphics = this;

    clone(&dxShader->Name, path.file_name());
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

void dx_graphics::swap() { D3DSwapChain->Present(TargetWindow->VSyncEnabled ? 1 : 0, 0); }

void dx_graphics::release() {
    D3DSwapChain->SetFullscreenState(false, null);

    SAFE_RELEASE(D3DDevice);
    SAFE_RELEASE(D3DDeviceContext);
    SAFE_RELEASE(D3DSwapChain);

    SAFE_RELEASE(D3DBackBuffer);

    SAFE_RELEASE(D3DDepthStencilBuffer);
    SAFE_RELEASE(D3DDepthStencilView);

    SAFE_RELEASE(D3DRasterState);

    SAFE_RELEASE(D3DBlendStates[0]);
    SAFE_RELEASE(D3DBlendStates[1]);
    SAFE_RELEASE(D3DDepthStencilStates[0]);
    SAFE_RELEASE(D3DDepthStencilStates[1]);
}

void dx_graphics::change_size(const window::window_resized_event &e) {
    SAFE_RELEASE(D3DBackBuffer);
    SAFE_RELEASE(D3DDepthStencilView);
    SAFE_RELEASE(D3DDepthStencilBuffer);

    DXCHECK(D3DSwapChain->ResizeBuffers(1, e.Width, e.Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    ID3D11Texture2D *swapChainBackBuffer;
    defer(SAFE_RELEASE(swapChainBackBuffer));

    DXCHECK(D3DSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &swapChainBackBuffer));
    DXCHECK(D3DDevice->CreateRenderTargetView(swapChainBackBuffer, null, &D3DBackBuffer));

    {
        D3D11_TEXTURE2D_DESC desc;
        zero_memory(&desc, sizeof(desc));
        {
            desc.Width = e.Width;
            desc.Height = e.Height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
        }
        DXCHECK(D3DDevice->CreateTexture2D(&desc, null, &D3DDepthStencilBuffer));
        D3DDevice->CreateDepthStencilView(D3DDepthStencilBuffer, 0, &D3DDepthStencilView);
    }
    D3DDeviceContext->OMSetRenderTargets(1, &D3DBackBuffer, D3DDepthStencilView);

    {
        D3D11_DEPTH_STENCIL_VIEW_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        {
            desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }
        DXCHECK(D3DDevice->CreateDepthStencilView(D3DDepthStencilBuffer, &desc, &D3DDepthStencilView));
    }
    D3DDeviceContext->OMSetRenderTargets(1, &D3DBackBuffer, D3DDepthStencilView);

    D3D11_VIEWPORT viewport;
    zero_memory(&viewport, sizeof(viewport));
    {
        viewport.Width = (f32) e.Width;
        viewport.Height = (f32) e.Height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
    }
    D3DDeviceContext->RSSetViewports(1, &viewport);

    {
        D3D11_RASTERIZER_DESC desc;
        zero_memory(&desc, sizeof(desc));
        {
            desc.AntialiasedLineEnable = false;
            desc.CullMode = D3D11_CULL_BACK;
            desc.DepthBias = 0;
            desc.DepthBiasClamp = 0.0f;
            desc.DepthClipEnable = true;
            desc.FillMode = D3D11_FILL_SOLID;
            desc.FrontCounterClockwise = false;
            desc.MultisampleEnable = false;
            desc.ScissorEnable = false;
            desc.SlopeScaledDepthBias = 0.0f;
        }
        DXCHECK(D3DDevice->CreateRasterizerState(&desc, &D3DRasterState));
    }
    D3DDeviceContext->RSSetState(D3DRasterState);
}
}  // namespace g
LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
