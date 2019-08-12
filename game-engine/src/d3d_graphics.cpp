#include "le/core.h"

#if OS == WINDOWS

#include <lstd/basic.h>

#include "le/graphics.h"

#undef MAC
#undef _MAC
#include <Windows.h>

#ifndef __dxgitype_h__
#undef DXGI_STATUS_OCCLUDED
#undef DXGI_STATUS_CLIPPED
#undef DXGI_STATUS_NO_REDIRECTION
#undef DXGI_STATUS_NO_DESKTOP_ACCESS
#undef DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_STATUS_MODE_CHANGED
#undef DXGI_STATUS_MODE_CHANGE_IN_PROGRESS
#undef DXGI_ERROR_INVALID_CALL
#undef DXGI_ERROR_NOT_FOUND
#undef DXGI_ERROR_MORE_DATA
#undef DXGI_ERROR_UNSUPPORTED
#undef DXGI_ERROR_DEVICE_REMOVED
#undef DXGI_ERROR_DEVICE_HUNG
#undef DXGI_ERROR_DEVICE_RESET
#undef DXGI_ERROR_WAS_STILL_DRAWING
#undef DXGI_ERROR_FRAME_STATISTICS_DISJOINT
#undef DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_ERROR_DRIVER_INTERNAL_ERROR
#undef DXGI_ERROR_NONEXCLUSIVE
#undef DXGI_ERROR_NOT_CURRENTLY_AVAILABLE
#undef DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED
#undef DXGI_ERROR_REMOTE_OUTOFMEMORY
#undef D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D11_ERROR_FILE_NOT_FOUND
#undef D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS
#undef D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD
#undef D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D10_ERROR_FILE_NOT_FOUND
#endif

#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dx10.h>
#include <d3dx11.h>

namespace le {

struct d3d_data {
    window *TargetWindow;

    ID3D11Device *Device;
    ID3D11DeviceContext *DeviceContext;
    IDXGISwapChain *SwapChain;

    ID3D11RenderTargetView *BackBuffer;

    ID3D11Texture2D *DepthStencilBuffer = null;
    ID3D11DepthStencilView *DepthStencilView = null;

    ID3D11RasterizerState *RasterState = null;

    ID3D11BlendState *BlendStates[2] = {null, null};
    ID3D11DepthStencilState *DepthStencilStates[2] = {null, null};
};

#define DD ((d3d_data *) D3DData)

void d3d_graphics::init(window *targetWindow) {
    static_assert(sizeof(D3DData) >= sizeof(d3d_data));  // Sanity

    DD->TargetWindow = targetWindow;

    IDXGIFactory *factory;
    defer(factory->Release());
    DXCHECK(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **) &factory));

    IDXGIAdapter *adapter;
    defer(adapter->Release());
    DXCHECK(factory->EnumAdapters(0, &adapter));

    IDXGIOutput *adapterOutput;
    defer(adapterOutput->Release());
    DXCHECK(adapter->EnumOutputs(0, &adapterOutput));

    u32 numModes;
    DXCHECK(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL));

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

    {
        DXGI_SWAP_CHAIN_DESC desc;
        zero_memory(&desc, sizeof(desc));
        {
            desc.BufferCount = 1;  // One back buffer
            desc.BufferDesc.Width = targetWindow->Width;
            desc.BufferDesc.Height = targetWindow->Height;
            desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Use 32-bit color
            desc.BufferDesc.RefreshRate.Numerator = targetWindow->VSyncEnabled ? numerator : 0;
            desc.BufferDesc.RefreshRate.Denominator = targetWindow->VSyncEnabled ? denominator : 1;
            desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;           // How the swap chain is going to be used
            desc.OutputWindow = *((HWND *) &targetWindow->PlatformData);  // The window to be used
            desc.SampleDesc.Count = 4;                                    // How many multisamples
            desc.Windowed = true;                                         // Windowed/full-screen mode
            // @TODO Full-screen, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
        }

        // In order to support Direct2D, pass D3D11_CREATE_DEVICE_BGRA_SUPPORT to _Flags_
        DXCHECK(D3D11CreateDeviceAndSwapChain(null, D3D_DRIVER_TYPE_HARDWARE, null, 0, null, 0, D3D11_SDK_VERSION,
                                              &desc, &DD->SwapChain, &DD->Device, null, &DD->DeviceContext));
    }

    {
        D3D11_BLEND_DESC desc;
        zero_memory(&desc, sizeof(desc));
        {
            desc.AlphaToCoverageEnable = FALSE;
            desc.IndependentBlendEnable = FALSE;

            desc.RenderTarget[0].BlendEnable = TRUE;
            desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        }
        DXCHECK(DD->Device->CreateBlendState(&desc, &DD->BlendStates[0]));

        zero_memory(&desc, sizeof(desc));
        {
            desc.RenderTarget[0].BlendEnable = FALSE;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        }
        DXCHECK(DD->Device->CreateBlendState(&desc, &DD->BlendStates[1]));
    }

    {
        D3D11_DEPTH_STENCIL_DESC desc;
        zero_memory(&desc, sizeof(desc));
        {
            desc.DepthEnable = TRUE;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
            desc.StencilEnable = TRUE;
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
        DXCHECK(DD->Device->CreateDepthStencilState(&desc, &DD->DepthStencilStates[0]));

        zero_memory(&desc, sizeof(desc));
        {
            desc.DepthEnable = FALSE;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
            desc.StencilEnable = TRUE;
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
        DXCHECK(DD->Device->CreateDepthStencilState(&desc, &DD->DepthStencilStates[1]));
    }

    change_size({targetWindow, targetWindow->Width, targetWindow->Height});

    set_blend(false);
    set_depth_testing(false);
}

void d3d_graphics::clear_color(vec4 color) {
    f32 c[] = {color.r, color.g, color.b, color.a};

    DD->DeviceContext->ClearRenderTargetView(DD->BackBuffer, c);
    DD->DeviceContext->ClearDepthStencilView(DD->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void d3d_graphics::set_blend(bool enabled) {
    DD->DeviceContext->OMSetBlendState(enabled ? DD->BlendStates[0] : DD->BlendStates[1], null, 0xffffffff);
}

void d3d_graphics::set_depth_testing(bool enabled) {
    DD->DeviceContext->OMSetDepthStencilState(enabled ? DD->DepthStencilStates[0] : DD->DepthStencilStates[1], 0);
}

void d3d_graphics::swap() { DD->SwapChain->Present(DD->TargetWindow->VSyncEnabled ? 1 : 0, 0); }

void d3d_graphics::release() {
    DD->Device->Release();
    DD->DeviceContext->Release();
    DD->SwapChain->Release();

    DD->BackBuffer->Release();

    DD->DepthStencilBuffer->Release();
    DD->DepthStencilView->Release();

    DD->RasterState->Release();

    DD->BlendStates[0]->Release();
    DD->BlendStates[1]->Release();
    DD->DepthStencilStates[0]->Release();
    DD->DepthStencilStates[1]->Release();
}

void d3d_graphics::change_size(const window_resized_event &e) {
    if (DD->BackBuffer) DD->BackBuffer->Release();
    if (DD->DepthStencilView) DD->DepthStencilView->Release();
    if (DD->DepthStencilBuffer) DD->DepthStencilBuffer->Release();

    DXCHECK(DD->SwapChain->ResizeBuffers(1, e.Width, e.Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    ID3D11Texture2D *swapChainBackBuffer;
    defer(swapChainBackBuffer->Release());

    DXCHECK(DD->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &swapChainBackBuffer));
    DXCHECK(DD->Device->CreateRenderTargetView(swapChainBackBuffer, NULL, &DD->BackBuffer));

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
        DXCHECK(DD->Device->CreateTexture2D(&desc, NULL, &DD->DepthStencilBuffer));
        DD->Device->CreateDepthStencilView(DD->DepthStencilBuffer, 0, &DD->DepthStencilView);
    }
    DD->DeviceContext->OMSetRenderTargets(1, &DD->BackBuffer, DD->DepthStencilView);

    {
        D3D11_DEPTH_STENCIL_VIEW_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        {
            desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }
        DXCHECK(DD->Device->CreateDepthStencilView(DD->DepthStencilBuffer, &desc, &DD->DepthStencilView));
    }
    DD->DeviceContext->OMSetRenderTargets(1, &DD->BackBuffer, DD->DepthStencilView);

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
    DD->DeviceContext->RSSetViewports(1, &viewport);

    {
        D3D11_RASTERIZER_DESC desc;
        zero_memory(&desc, sizeof(desc));
        {
            desc.AntialiasedLineEnable = false;
            desc.CullMode = D3D11_CULL_BACK;
            desc.DepthBias = 0;
            desc.DepthBiasClamp = 0.0f;
            desc.DepthClipEnable = TRUE;
            desc.FillMode = D3D11_FILL_SOLID;
            desc.FrontCounterClockwise = FALSE;
            desc.MultisampleEnable = FALSE;
            desc.ScissorEnable = FALSE;
            desc.SlopeScaledDepthBias = 0.0f;
            desc.FrontCounterClockwise = TRUE;
        }
        DXCHECK(DD->Device->CreateRasterizerState(&desc, &DD->RasterState));
    }
    DD->DeviceContext->RSSetState(DD->RasterState);
}

}  // namespace le

#endif  // OS == WINDOWS
