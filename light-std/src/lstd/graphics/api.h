#pragma once

/// Defines the graphics API that can be used to draw stuff on windows.
/// Implementations of it can be switched dynamically.

#include "../file.h"
#include "../math.h"
#include "../video.h"
#include "buffer.h"
#include "shader.h"
#include "texture.h"

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

enum class graphics_api {
    None = 0,
#if OS == WINDOWS
    Direct3D  // We can't run D3D on anything other than Windows (or Xbox but we don't support consoles yet)
#endif
};

enum class cull : u32 { None = 0, Front, Back };

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
        size_t ClosedCallbackID = npos, FramebufferResizedCallbackID = npos;

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
    void set_target_window(window *win) {
        size_t index = TargetWindows.find([&](auto x) { return x.Window == win; });
        target_window *targetWindow;
        if (index == npos) {
            targetWindow = TargetWindows.append();
            targetWindow->Window = win;
            if (win) {
                targetWindow->ClosedCallbackID = win->WindowClosedEvent.connect({this, &graphics::window_closed});
                targetWindow->FramebufferResizedCallbackID =
                    win->WindowFramebufferResizedEvent.connect({this, &graphics::window_resized});
                Impl.InitTargetWindow(this, targetWindow);
                window_resized({win, win->get_size().x, win->get_size().y});
            }
        } else {
            targetWindow = &TargetWindows[index];
        }

        CurrentTargetWindow = targetWindow;
        if (win) set_custom_render_target(targetWindow->CustomRenderTarget);
    }

    rect get_viewport() {
        assert(CurrentTargetWindow->Window);
        return CurrentTargetWindow->Viewport;
    }

    void set_viewport(rect viewport) {
        assert(CurrentTargetWindow->Window);
        CurrentTargetWindow->Viewport = viewport;
        Impl.SetViewport(this, viewport);
    }

    rect get_scissor_rect() {
        assert(CurrentTargetWindow->Window);
        return CurrentTargetWindow->ScissorRect;
    }

    void set_scissor_rect(rect scissorRect) {
        assert(CurrentTargetWindow->Window);
        CurrentTargetWindow->ScissorRect = scissorRect;
        Impl.SetScissorRect(this, scissorRect);
    }

    // Pass _null_ to restore rendering to the back buffer
    void set_custom_render_target(texture_2D *target) {
        assert(CurrentTargetWindow->Window);
        CurrentTargetWindow->CustomRenderTarget = target;
        Impl.SetRenderTarget(this, target);

        set_cull_mode(CurrentTargetWindow->CullMode);

        vec2<s32> size = CurrentTargetWindow->Window->get_size();
        if (target) size = {target->Width, target->Height};
        set_viewport({0, 0, size.x, size.y});
        set_scissor_rect({0, 0, size.x, size.y});
    }

    void set_blend(bool enabled) { Impl.SetBlend(this, enabled); }
    void set_depth_testing(bool enabled) { Impl.SetDepthTesting(this, enabled); }

    void set_cull_mode(cull mode) {
        assert(CurrentTargetWindow->Window);
        CurrentTargetWindow->CullMode = mode;
        Impl.SetCullMode(this, mode);
    }

    void clear_color(v4 color) {
        assert(CurrentTargetWindow->Window);
        if (!CurrentTargetWindow->Window->is_visible()) return;

        Impl.ClearColor(this, color);
    }

    void draw(u32 vertices, u32 startVertexLocation = 0) { Impl.Draw(this, vertices, startVertexLocation); }
    void draw_indexed(u32 indices, u32 startIndex = 0, u32 baseVertexLocation = 0) {
        Impl.DrawIndexed(this, indices, startIndex, baseVertexLocation);
    }

    void swap() {
        assert(CurrentTargetWindow->Window);
        if (!CurrentTargetWindow->Window->is_visible()) return;

        Impl.Swap(this);
    }

    void release() {
        if (Impl.Release) {
            For_as(it_index, range(TargetWindows.Count)) {
                auto *it = &TargetWindows[it_index];
                if (it->Window) {
                    it->Window->WindowClosedEvent.disconnect(it->ClosedCallbackID);
                    it->Window->WindowFramebufferResizedEvent.disconnect(it->FramebufferResizedCallbackID);
                    Impl.ReleaseTargetWindow(this, it);
                }
            }
            TargetWindows.reset();

            Impl.Release(this);

            API = graphics_api::None;
        }
        assert(API == graphics_api::None);
    }

   private:
    void window_closed(const window_closed_event &e) {
        size_t index = TargetWindows.find([&](auto x) { return x.Window == e.Window; });
        assert(index != npos);

        target_window *targetWindow = &TargetWindows[index];
        targetWindow->Window->WindowClosedEvent.disconnect(targetWindow->ClosedCallbackID);
        targetWindow->Window->WindowFramebufferResizedEvent.disconnect(targetWindow->FramebufferResizedCallbackID);
        Impl.ReleaseTargetWindow(this, targetWindow);

        TargetWindows.remove(index);
    }

    void window_resized(const window_framebuffer_resized_event &e) {
        size_t index = TargetWindows.find([&](auto x) { return x.Window == e.Window; });
        assert(index != npos);

        if (!e.Window->is_visible()) return;
        Impl.TargetWindowResized(this, &TargetWindows[index], e.Width, e.Height);
    }
};

LSTD_END_NAMESPACE
