#pragma once

#include "graphics/graphics.h"

#if OS == WINDOWS
struct ID3D11Buffer;
struct D3D11_MAPPED_SUBRESOURCE;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct IDXGIFactory;
struct IDXGIAdapter;
struct IDXGIDevice;

LSTD_BEGIN_NAMESPACE

struct dx_graphics;

struct dx_texture_2D : public texture_2D {
    dx_graphics *D3DGraphics = null;
    ID3D11Texture2D *D3DTexture;
    ID3D11ShaderResourceView *D3DResourceView;
    ID3D11SamplerState *D3DSamplerState;

    dx_texture_2D() = default;
    ~dx_texture_2D() { release(); }

    void bind(u32 slot) override;

// For debugging purposes (to reset state), shouldn't use this in Dist
#if defined DEBUG || defined RELEASE
    void unbind(u32 slot) override;
#endif

    void set_data(const char *pixels) override;
    void set_data(u32 color) override;

    void release() override;
};

struct dx_buffer : public buffer {
    dx_graphics *D3DGraphics = null;
    ID3D11Buffer *D3DBuffer = null;
    ID3D11InputLayout *D3DLayout = null;

    // Based on the definition of _D3D11_MAPPED_SUBRESOURCE_
    char MappedData[POINTER_SIZE + sizeof(u32) + sizeof(u32)];

    dx_buffer() = default;
    ~dx_buffer() { release(); }

    void set_input_layout(buffer_layout *layout) override;

    void *map(map_access access) override;
    void unmap() override;

    void bind(bind_data bindData) override;

#if defined DEBUG || defined RELEASE
    void unbind() override;
#endif

    void release() override;
};

struct dx_shader : public shader {
    dx_graphics *D3DGraphics = null;

    struct d3d_data {
        ID3D11VertexShader *VS = null;
        ID3D11PixelShader *PS = null;
        // We can't forward declare _ID3DBlob_
        void *VSBlob = null;
        void *PSBlob = null;
    };
    d3d_data D3DData;

    dx_shader() = default;
    ~dx_shader() { release(); };

    void bind() override;

// For debugging purposes (to reset state), shouldn't use this in Dist
#if defined DEBUG || defined RELEASE
    void unbind() override;
#endif

    void release() override;
};

struct dx_graphics : public graphics {
    ID3D11Device *D3DDevice = null;
    ID3D11DeviceContext *D3DDeviceContext = null;

    struct target_window {
        window *Window = null;
        size_t ResizeCallbackID = 0;

        IDXGISwapChain *D3DSwapChain = null;

        ID3D11RenderTargetView *D3DBackBuffer = null;

        ID3D11Texture2D *D3DDepthStencilBuffer = null;
        ID3D11DepthStencilView *D3DDepthStencilView = null;

        cull CullMode = cull::None;
        ID3D11RasterizerState *D3DRasterState[3] = {null, null};
    };
    array<target_window> TargetWindows;
    target_window *CurrentTargetWindow = null;

    ID3D11BlendState *D3DBlendStates[2] = {null, null};
    ID3D11DepthStencilState *D3DDepthStencilStates[2] = {null, null};

    dx_shader *D3DBoundShader = null;

    dx_graphics() = default;
    ~dx_graphics() { release(); }

    void init() override;

    void add_target_window(window *win) override;
    void remove_target_window(window *win) override;
    void set_current_target_window(window *win) override;

    void clear_color(vec4 color) override;

    void set_blend(bool enabled) override;
    void set_depth_testing(bool enabled) override;
    void set_cull_mode(cull mode) override;

    void create_buffer(buffer *buffer, buffer::type type, buffer::usage usage, size_t size) override;
    void create_buffer(buffer *buffer, buffer::type type, buffer::usage usage, const char *initialData,
                       size_t size) override;

    void create_shader(shader *shader, string name, file::path path) override;

    void create_texture_2D(texture_2D *texture, string name, u32 width, u32 height,
                           texture::filter filter = texture::LINEAR, texture::wrap wrap = texture::CLAMP) override;

    void create_texture_2D_from_file(texture_2D *texture, string name, file::path path, bool flipX = false,
                                     bool flipY = false, texture::filter filter = texture::LINEAR,
                                     texture::wrap wrap = texture::CLAMP) override;

    void draw(size_t vertices) override;
    void draw_indexed(size_t indices) override;

    void swap() override;

    void release() override;

   private:
    void window_changed_size(const window_framebuffer_resized_event &e);
};

#endif

LSTD_END_NAMESPACE
