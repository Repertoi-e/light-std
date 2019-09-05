#include "lstd/common.h"

#if OS == WINDOWS

#include "lstd/graphics/api.h"
#include "lstd/os.h"

#undef MAC
#undef _MAC
#include <Windows.h>

#include <d3d10_1.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgidebug.h>

LSTD_BEGIN_NAMESPACE

void d3d_texture_2D_init(texture_2D *t) {
    D3D11_TEXTURE2D_DESC textureDesc;
    zero_memory(&textureDesc, sizeof(textureDesc));
    {
        textureDesc.Width = t->Width;
        textureDesc.Height = t->Height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Usage = D3D11_USAGE_DYNAMIC;
        textureDesc.CPUAccessFlags = textureDesc.Usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (t->RenderTarget ? D3D11_BIND_RENDER_TARGET : 0);
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
    }
    DXCHECK(t->Graphics->D3D.Device->CreateTexture2D(&textureDesc, null, &t->D3D.Texture));

    D3D11_SHADER_RESOURCE_VIEW_DESC rvDesc;
    zero_memory(&rvDesc, sizeof(rvDesc));
    {
        rvDesc.Format = textureDesc.Format;
        rvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        rvDesc.Texture2D.MipLevels = 1;
    }
    DXCHECK(t->Graphics->D3D.Device->CreateShaderResourceView(t->D3D.Texture, &rvDesc, &t->D3D.ResourceView));

    if (t->RenderTarget) {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        zero_memory(&rtvDesc, sizeof(rtvDesc));
        {
            rtvDesc.Format = textureDesc.Format;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        }
        DXCHECK(t->Graphics->D3D.Device->CreateRenderTargetView(t->D3D.Texture, &rtvDesc, &t->D3D.RenderTargetView));
    }

    D3D11_TEXTURE_ADDRESS_MODE addressMode;
    if (t->Wrap == texture_wrap::Clamp) addressMode = D3D11_TEXTURE_ADDRESS_CLAMP;
    if (t->Wrap == texture_wrap::Mirrored_Repeat) addressMode = D3D11_TEXTURE_ADDRESS_MIRROR;
    if (t->Wrap == texture_wrap::Repeat) addressMode = D3D11_TEXTURE_ADDRESS_WRAP;
    if (t->Wrap == texture_wrap::Clamp_To_Border) addressMode = D3D11_TEXTURE_ADDRESS_BORDER;

    D3D11_SAMPLER_DESC samplerDesc;
    zero_memory(&samplerDesc, sizeof(samplerDesc));
    {
        samplerDesc.AddressU = addressMode;
        samplerDesc.AddressV = addressMode;
        samplerDesc.AddressW = addressMode;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = 11;
        samplerDesc.Filter =
            t->Filter == texture_filter::Linear ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    }
    DXCHECK(t->Graphics->D3D.Device->CreateSamplerState(&samplerDesc, &t->D3D.SamplerState));
}

void d3d_texture_2D_set_data(texture_2D *t, pixel_buffer data) {
    // We have a very strict image format and don't support anything else at the moment...
    assert(t->Width == data.Width && t->Height == data.Height && data.BPP == 4);

    D3D11_MAPPED_SUBRESOURCE mappedData;
    zero_memory(&mappedData, sizeof(mappedData));

    DXCHECK(t->Graphics->D3D.DeviceContext->Map(t->D3D.Texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    copy_memory(mappedData.pData, data.Pixels, t->Width * t->Height * 4);
    t->Graphics->D3D.DeviceContext->Unmap(t->D3D.Texture, 0);
}

void d3d_texture_2D_bind(texture_2D *t) {
    assert(t->BoundSlot != (u32) -1);

    t->Graphics->D3D.DeviceContext->PSSetShaderResources(t->BoundSlot, 1, &t->D3D.ResourceView);
    t->Graphics->D3D.DeviceContext->PSSetSamplers(t->BoundSlot, 1, &t->D3D.SamplerState);
}

void d3d_texture_2D_unbind(texture_2D *t) {
    assert(t->BoundSlot != (u32) -1);

    ID3D11ShaderResourceView *rv = null;
    t->Graphics->D3D.DeviceContext->PSSetShaderResources(t->BoundSlot, 1, &rv);
}

void d3d_texture_2D_release(texture_2D *t) {
    SAFE_RELEASE(t->D3D.Texture);
    SAFE_RELEASE(t->D3D.ResourceView);
    SAFE_RELEASE(t->D3D.SamplerState);
    SAFE_RELEASE(t->D3D.RenderTargetView);
}

texture_2D::impl g_D3DTexture2DImpl = {d3d_texture_2D_init, d3d_texture_2D_set_data, d3d_texture_2D_bind,
                                       d3d_texture_2D_unbind, d3d_texture_2D_release};

LSTD_END_NAMESPACE

#endif
