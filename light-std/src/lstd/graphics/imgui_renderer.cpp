#include "imgui_renderer.h"

#include "../memory/pixel_buffer.h"
#include "../video/window.h"

LSTD_BEGIN_NAMESPACE

void imgui_renderer::init(graphics *g) {
    assert(!Graphics);
    Graphics = g;

    ImGuiIO &io = ImGui::GetIO();
    io.BackendRendererName = "lstd";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

    ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
    platformIO.Renderer_RenderWindow = [](auto *viewport, void *context) {
        if (!((window *) viewport->PlatformHandle)->is_visible()) return;

        auto *renderer = (imgui_renderer *) context;
        if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear)) {
            renderer->Graphics->clear_color(v4(0.0f, 0.0f, 0.0f, 1.0f));
        }
        renderer->draw(viewport->DrawData);
    };

    Shader.Name = "UI Shader";
    Shader.init(g, file::path("data/UI.hlsl"));

    UB.init(g, buffer_type::Shader_Uniform_Buffer, buffer_usage::Dynamic, sizeof(mat<f32, 4, 4>));

    s32 width, height;
    u8 *pixels = null;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    FontTexture.init(g, width, height);
    FontTexture.set_data(pixel_buffer(pixels, width, height, pixel_format::RGBA));

    io.Fonts->TexID = &FontTexture;
}

void imgui_renderer::draw(ImDrawData *drawData) {
    if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f) return;

    if (VBSize < drawData->TotalVtxCount) {
        VB.release();

        VBSize = drawData->TotalVtxCount + 5000;
        VB.init(Graphics, buffer_type::Vertex_Buffer, buffer_usage::Dynamic, VBSize * sizeof(ImDrawVert));

        Shader.bind();
        buffer_layout layout;
        layout.add("POSITION", gtype::F32_2);
        layout.add("TEXCOORD", gtype::F32_2);
        layout.add("COLOR", gtype::U32, 1, true);
        VB.set_input_layout(layout);
    }

    if (IBSize < drawData->TotalIdxCount) {
        IBSize = drawData->TotalIdxCount + 10000;
        IB.init(Graphics, buffer_type::Index_Buffer, buffer_usage::Dynamic, IBSize * sizeof(u32));
    }

    auto *vb = (ImDrawVert *) VB.map(buffer_map_access::Write_Discard_Previous);
    auto *ib = (u32 *) IB.map(buffer_map_access::Write_Discard_Previous);

    For_as(it_index, range(drawData->CmdListsCount)) {
        auto *it = drawData->CmdLists[it_index];
        copy_memory(vb, it->VtxBuffer.Data, it->VtxBuffer.Size * sizeof(ImDrawVert));
        copy_memory(ib, it->IdxBuffer.Data, it->IdxBuffer.Size * sizeof(u32));
        vb += it->VtxBuffer.Size;
        ib += it->IdxBuffer.Size;
    }
    VB.unmap();
    IB.unmap();

    auto *ub = UB.map(buffer_map_access::Write_Discard_Previous);
    f32 L = drawData->DisplayPos.x;
    f32 R = drawData->DisplayPos.x + drawData->DisplaySize.x;
    f32 T = drawData->DisplayPos.y;
    f32 B = drawData->DisplayPos.y + drawData->DisplaySize.y;
    f32 mvp[4][4] = {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, 0.5f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
    };
    copy_memory(ub, &mvp, sizeof(mvp));
    UB.unmap();

    set_render_state();

    rect oldScissorRect = Graphics->get_scissor_rect();

    s32 vtxOffset = 0, idxOffset = 0;
    For_as(cmdListIndex, range(drawData->CmdListsCount)) {
        auto *cmdList = drawData->CmdLists[cmdListIndex];
        For(cmdList->CmdBuffer) {
            if (it.UserCallback) {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer
                // to reset render state.)
                if (it.UserCallback == ImDrawCallback_ResetRenderState) {
                    set_render_state();
                } else {
                    it.UserCallback(cmdList, &it);
                }
            } else {
                s32 left = (s32)(it.ClipRect.x - drawData->DisplayPos.x);
                s32 top = (s32) (it.ClipRect.y - drawData->DisplayPos.y);
                s32 right = (s32)(it.ClipRect.z - drawData->DisplayPos.x);
                s32 bot = (s32)(it.ClipRect.w - drawData->DisplayPos.y);
                Graphics->set_scissor_rect({left, top, right, bot});

                if (it.TextureId) ((texture_2D *) it.TextureId)->bind(0);
                Graphics->draw_indexed(it.ElemCount, it.IdxOffset + idxOffset, it.VtxOffset + vtxOffset);
            }
        }
        idxOffset += cmdList->IdxBuffer.Size;
        vtxOffset += cmdList->VtxBuffer.Size;
    }
    Graphics->set_scissor_rect(oldScissorRect);
}

void imgui_renderer::release() {
    VB.release();
    IB.release();
    UB.release();
    ImGui::GetIO().Fonts->TexID = null;
    FontTexture.release();
    Shader.release();
}

void imgui_renderer::set_render_state() {
    Shader.bind();
    VB.bind_vb(primitive_topology::TriangleList);
    IB.bind_ib();
    UB.bind_ub(shader_type::Vertex_Shader, 0);
}

LSTD_END_NAMESPACE
