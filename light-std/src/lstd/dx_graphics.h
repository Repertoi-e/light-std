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

LSTD_BEGIN_NAMESPACE

namespace g {

struct dx_graphics;

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
    window::window *TargetWindow = null;

    ID3D11Device *D3DDevice = null;
    ID3D11DeviceContext *D3DDeviceContext = null;
    IDXGISwapChain *D3DSwapChain = null;

    ID3D11RenderTargetView *D3DBackBuffer = null;

    ID3D11Texture2D *D3DDepthStencilBuffer = null;
    ID3D11DepthStencilView *D3DDepthStencilView = null;

    ID3D11RasterizerState *D3DRasterState = null;

    ID3D11BlendState *D3DBlendStates[2] = {null, null};
    ID3D11DepthStencilState *D3DDepthStencilStates[2] = {null, null};

    dx_shader *D3DBoundShader = null;

    dx_graphics() = default;
    ~dx_graphics() { release(); }

    void init(window::window *targetWindow) override;

    void clear_color(vec4 color) override;

    void set_blend(bool enabled) override;
    void set_depth_testing(bool enabled) override;

    void create_buffer(buffer *buffer, buffer::type type, buffer::usage usage, size_t size) override;
    void create_buffer(buffer *buffer, buffer::type type, buffer::usage usage, const char *initialData,
                       size_t size) override;

    void create_shader(shader *shader, file::path path) override;

    void draw(size_t vertices) override;
    void draw_indexed(size_t indices) override;

    void swap() override;

    void release() override;

   private:
    void change_size(const window::window_resized_event &e);
};
}  // namespace g
#endif

LSTD_END_NAMESPACE
