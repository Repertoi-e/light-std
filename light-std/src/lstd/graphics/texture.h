#pragma once

#include "asset.h"
#include "gtype.h"

#if OS == WINDOWS
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
#endif

LSTD_BEGIN_NAMESPACE

enum class texture_wrap { None = 0,
                          Repeat,
                          Clamp,
                          Mirrored_Repeat,
                          Clamp_To_Border };
enum class texture_filter { Linear,
                            Nearest };

// @AvoidInclude
struct pixel_buffer;
struct graphics;

struct texture_2D : asset {
#if OS == WINDOWS
    struct {
        ID3D11Texture2D *Texture = null;
        ID3D11ShaderResourceView *ResourceView = null;
        ID3D11SamplerState *SamplerState = null;

        ID3D11RenderTargetView *RenderTargetView = null;
        ID3D11Texture2D *DepthStencilBuffer = null;
        ID3D11DepthStencilView *DepthStencilView = null;
    } D3D{};
#endif

    struct impl {
        void (*Init)(texture_2D *t) = null;
        void (*SetData)(texture_2D *t, pixel_buffer data) = null;

        void (*Bind)(texture_2D *t) = null;
        void (*Unbind)(texture_2D *t) = null;
        void (*Release)(texture_2D *t) = null;
    } Impl{};

    graphics *Graphics;

    s32 Width = 0;
    s32 Height = 0;

    u32 BoundSlot = (u32) -1;

    texture_wrap Wrap;
    texture_filter Filter;
    bool RenderTarget = false;  // When true, the texture can be used as a framebuffer

    texture_2D() = default;

    // We no longer use destructors for deallocation.
    // ~texture_2D() { release(); }

    void init(graphics *g, s32 width, s32 height, texture_filter filter = texture_filter::Linear,
              texture_wrap wrap = texture_wrap::Clamp);
    void init_as_render_target(graphics *g, s32 width, s32 height, texture_filter filter = texture_filter::Linear,
                               texture_wrap wrap = texture_wrap::Repeat);

    void set_data(pixel_buffer data);

    void bind(u32 slot);
    void unbind();

    void release();
};

LSTD_END_NAMESPACE
