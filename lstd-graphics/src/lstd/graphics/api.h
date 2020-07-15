#pragma once

/// Defines the graphics API that can be used to draw stuff on windows.
/// Implementations of it can be switched dynamically.

#include "lstd/math/rect.h"
#include "lstd/math/vec.h"
#include "lstd/memory/array.h"

#if OS == WINDOWS
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11RasterizerState;
#endif

LSTD_BEGIN_NAMESPACE

// @AvoidInclude
struct window;
struct event;

struct shader;
struct texture_2D;

enum class graphics_api {
    None = 0,
#if OS == WINDOWS
    Direct3D  // We can't run D3D on anything other than Windows (or Xbox but we don't support consoles yet)
#endif
};

enum class cull : u32 { None = 0,
                        Front,
                        Back };

struct graphics : non_copyable, non_movable {
#if OS == WINDOWS
    struct {
        ID3D11Device *Device = null;
        ID3D11DeviceContext *DeviceContext = null;

        ID3D11BlendState *BlendStates[2] = {null, null};
        ID3D11DepthStencilState *DepthStencilStates[2] = {null, null};
    } D3D{};
#endif

    struct target_window {
        window *Window = null;
        s64 CallbackID = -1;

        cull CullMode = cull::None;
        rect Viewport, ScissorRect;

        texture_2D *CustomRenderTarget = null;

#if OS == WINDOWS
        struct {
            IDXGISwapChain *SwapChain = null;

            ID3D11RenderTargetView *BackBuffer = null;
            ID3D11RenderTargetView *RenderTarget =
                null;  // Normally set to _BackBuffer_, you may specify a
                       // different render target (e.g. a framebuffer texture by calling _set_custom_render_target_)

            ID3D11Texture2D *DepthStencilBuffer = null;
            ID3D11DepthStencilView *DepthStencilView = null;

            ID3D11RasterizerState *RasterStates[3] = {null, null};
        } D3D{};
#endif
    };
    array<target_window> TargetWindows;
    target_window *CurrentTargetWindow = null;

    shader *CurrentlyBoundShader = null;

    graphics_api API = graphics_api::None;

    graphics() = default;
    ~graphics() { release(); }

    struct impl {
        void (*Init)(graphics *g) = null;

        void (*InitTargetWindow)(graphics *g, target_window *targetWindow) = null;
        void (*ReleaseTargetWindow)(graphics *g, target_window *targetWindow) = null;
        void (*TargetWindowResized)(graphics *g, target_window *targetWindow, s32 width, s32 height) = null;

        void (*SetViewport)(graphics *g, rect viewport) = null;
        void (*SetScissorRect)(graphics *g, rect scissorRect) = null;

        void (*SetRenderTarget)(graphics *g, texture_2D *target) = null;  // target == null means back buffer

        void (*SetBlend)(graphics *g, bool enabled) = null;
        void (*SetDepthTesting)(graphics *g, bool enabled) = null;
        void (*SetCullMode)(graphics *g, cull mode) = null;

        void (*ClearColor)(graphics *g, v4 color) = null;
        void (*Draw)(graphics *g, u32 vertices, u32 startVertexLocation) = null;
        void (*DrawIndexed)(graphics *g, u32 indices, u32 startIndex, u32 baseVertexLocation) = null;
        void (*Swap)(graphics *g) = null;
        void (*Release)(graphics *g) = null;
    } Impl{};

    void init(graphics_api api);

    // Sets the current render context, so you can draw to multiple windows using the same _graphics_ object.
    // If you want to draw to a texture, use _set_custom_render_target_, note that you must still have a valid
    // target window, and that window is associated with the resources which get created.
    void set_target_window(window *win);

    rect get_viewport();
    void set_viewport(rect viewport);

    rect get_scissor_rect();
    void set_scissor_rect(rect scissorRect);

    // Pass _null_ to restore rendering to the back buffer
    void set_custom_render_target(texture_2D *target);

    void set_blend(bool enabled);
    void set_depth_testing(bool enabled);

    void set_cull_mode(cull mode);

    void clear_color(v4 color);

    void draw(u32 vertices, u32 startVertexLocation = 0);
    void draw_indexed(u32 indices, u32 startIndex = 0, u32 baseVertexLocation = 0);

    void swap();

    void release();

   private:
    bool window_event_handler(const event &e);
};

LSTD_END_NAMESPACE
