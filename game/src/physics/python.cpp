#include "state.h"

static u32 rgb_to_imgui(u32 x) {
    auto r = (x >> 16) & 0xFF;
    auto g = (x >> 8) & 0xFF;
    auto b = (x >> 0) & 0xFF;
    return IM_COL32(r, g, b, 0xff);
}

PYBIND11_MODULE(lstdgraphics, m) {
    m.doc() = "A module which exposes 2D draw functions from our graphics engine to python and interlops with C++.";

    m.def(
        "state",
        [](u64 pointer) {
            GameState = (game_state *) pointer;

            // Switch our default allocator from malloc to the one the exe provides us with
            Context.Alloc = GameState->Memory->Alloc;

        // @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack
#if defined DEBUG_MEMORY
            allocator::DEBUG_Head = GameState->DEBUG_Head;
            clone(&allocator::DEBUG_Mutex, *GameState->DEBUG_Mutex);
#endif
            allocator::AllocationCount = GameState->AllocationCount;

            // I'm not sure if we need this, but just incase
            assert(GameState->Memory->ImGuiContext);
            ImGui::SetCurrentContext((ImGuiContext *) GameState->Memory->ImGuiContext);

            // We also tell imgui to use our allocator
            ImGui::SetAllocatorFunctions([](size_t size, void *) { return operator new(size); },
                                         [](void *ptr, void *) { delete ptr; });
        },
        py::arg("pointer"));

    m.def(
        "line",
        [](py::array_t<f64> p1, py::array_t<f64> p2, u32 color, f64 thickness) {
            assert(GameState && "State not initialized");
            assert(p1.request().size == 2 && "line requires p1 to be an array of size 2");
            assert(p2.request().size == 2 && "line requires p2 to be an array of size 2");

            GameState->ViewportDrawlist->AddLine(v2(p1.at(0), p1.at(1)) * GameState->PixelsPerMeter,
                                                 v2(p2.at(0), p2.at(1)) * GameState->PixelsPerMeter,
                                                 rgb_to_imgui(color), (f32) thickness);
        },
        py::arg("p1"), py::arg("p2"), py::arg("color"), py::arg("thickness") = 1.0);

    py::enum_<ImDrawCornerFlags_>(m, "Corner", py::arithmetic(),
                                  "Used for specifying properties when drawing rectangles")
        .value("NONE", ImDrawCornerFlags_None, "No rounding")
        .value("TOP_LEFT", ImDrawCornerFlags_TopLeft, "Rounding top left")
        .value("TOP_RIGHT", ImDrawCornerFlags_TopRight, "Rounding top right")
        .value("BOT_LEFT", ImDrawCornerFlags_BotLeft, "Rounding bot left")
        .value("BOT_RIGHT", ImDrawCornerFlags_BotRight, "Rounding bot right")
        .value("TOP", ImDrawCornerFlags_Top, "Rounding top")
        .value("BOT", ImDrawCornerFlags_Bot, "Rounding bot")
        .value("LEFT", ImDrawCornerFlags_Left, "Rounding left")
        .value("RIGHT", ImDrawCornerFlags_Right, "Rounding right")
        .value("ALL", ImDrawCornerFlags_All, "Rounding all")
        .export_values();

    m.def(
        "rect",
        [](py::array_t<f64> p1, py::array_t<f64> p2, u32 color, f64 rounding, ImDrawCornerFlags_ cornerFlags,
           f64 thickness) {
            assert(GameState && "State not initialized");
            assert(p1.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p2.request().size == 2 && "rect requires p1 to be an array of size 2");
            GameState->ViewportDrawlist->AddRect(v2(p1.at(0), p1.at(1)) * GameState->PixelsPerMeter,
                                                 v2(p2.at(0), p2.at(1)) * GameState->PixelsPerMeter,
                                                 rgb_to_imgui(color), (f32) rounding, cornerFlags, (f32) thickness);
        },
        py::arg("p1"), py::arg("p2"), py::arg("color"), py::arg("rounding") = 0.0,
        py::arg("corner_flags") = ImDrawCornerFlags_None, py::arg("thickness") = 1.0);

    m.def(
        "rect_filled",
        [](py::array_t<f64> p1, py::array_t<f64> p2, u32 color, f64 rounding, ImDrawCornerFlags_ cornerFlags) {
            assert(GameState && "State not initialized");
            assert(p1.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p2.request().size == 2 && "rect requires p1 to be an array of size 2");
            GameState->ViewportDrawlist->AddRectFilled(v2(p1.at(0), p1.at(1)) * GameState->PixelsPerMeter,
                                                       v2(p2.at(0), p2.at(1)) * GameState->PixelsPerMeter,
                                                       rgb_to_imgui(color), (f32) rounding, cornerFlags);
        },
        py::arg("p1"), py::arg("p2"), py::arg("color"), py::arg("rounding") = 0.0,
        py::arg("corner_flags") = ImDrawCornerFlags_None);

    m.def(
        "rect_filled_multi_color",
        [](py::array_t<f64> p1, py::array_t<f64> p2, u32 color_ul, u32 color_ur, u32 color_dr, u32 color_dl) {
            assert(GameState && "State not initialized");
            assert(p1.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p2.request().size == 2 && "rect requires p1 to be an array of size 2");
            GameState->ViewportDrawlist->AddRectFilledMultiColor(
                v2(p1.at(0), p1.at(1)) * GameState->PixelsPerMeter, v2(p2.at(0), p2.at(1)) * GameState->PixelsPerMeter,
                rgb_to_imgui(color_ul), rgb_to_imgui(color_ur), rgb_to_imgui(color_dr), rgb_to_imgui(color_dl));
        },
        py::arg("p1"), py::arg("p2"), py::arg("color_ul"), py::arg("color_ur"), py::arg("color_dr"),
        py::arg("color_dl"));

    m.def(
        "quad",
        [](py::array_t<f64> p1, py::array_t<f64> p2, py::array_t<f64> p3, py::array_t<f64> p4, u32 color,
           f64 thickness) {
            assert(GameState && "State not initialized");
            assert(p1.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p2.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p3.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p4.request().size == 2 && "rect requires p1 to be an array of size 2");
            GameState->ViewportDrawlist->AddQuad(
                v2(p1.at(0), p1.at(1)) * GameState->PixelsPerMeter, v2(p2.at(0), p2.at(1)) * GameState->PixelsPerMeter,
                v2(p3.at(0), p3.at(1)) * GameState->PixelsPerMeter, v2(p4.at(0), p4.at(1)) * GameState->PixelsPerMeter,
                rgb_to_imgui(color), (f32) thickness);
        },
        py::arg("p1"), py::arg("p2"), py::arg("p3"), py::arg("p4"), py::arg("color"), py::arg("thickness") = 1.0);

    m.def(
        "quad_filled",
        [](py::array_t<f64> p1, py::array_t<f64> p2, py::array_t<f64> p3, py::array_t<f64> p4, u32 color) {
            assert(GameState && "State not initialized");
            assert(p1.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p2.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p3.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p4.request().size == 2 && "rect requires p1 to be an array of size 2");
            GameState->ViewportDrawlist->AddQuadFilled(
                v2(p1.at(0), p1.at(1)) * GameState->PixelsPerMeter, v2(p2.at(0), p2.at(1)) * GameState->PixelsPerMeter,
                v2(p3.at(0), p3.at(1)) * GameState->PixelsPerMeter, v2(p4.at(0), p4.at(1)) * GameState->PixelsPerMeter,
                rgb_to_imgui(color));
        },
        py::arg("p1"), py::arg("p2"), py::arg("p3"), py::arg("p4"), py::arg("color"));

    m.def(
        "triangle",
        [](py::array_t<f64> p1, py::array_t<f64> p2, py::array_t<f64> p3, u32 color, f64 thickness) {
            assert(GameState && "State not initialized");
            assert(p1.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p2.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p3.request().size == 2 && "rect requires p1 to be an array of size 2");
            GameState->ViewportDrawlist->AddTriangle(
                v2(p1.at(0), p1.at(1)) * GameState->PixelsPerMeter, v2(p2.at(0), p2.at(1)) * GameState->PixelsPerMeter,
                v2(p3.at(0), p3.at(1)) * GameState->PixelsPerMeter, rgb_to_imgui(color), (f32) thickness);
        },
        py::arg("p1"), py::arg("p2"), py::arg("p3"), py::arg("color"), py::arg("thickness") = 1.0);

    m.def(
        "triangle_filled",
        [](py::array_t<f64> p1, py::array_t<f64> p2, py::array_t<f64> p3, u32 color) {
            assert(GameState && "State not initialized");
            assert(p1.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p2.request().size == 2 && "rect requires p1 to be an array of size 2");
            assert(p3.request().size == 2 && "rect requires p1 to be an array of size 2");
            GameState->ViewportDrawlist->AddTriangleFilled(
                v2(p1.at(0), p1.at(1)) * GameState->PixelsPerMeter, v2(p2.at(0), p2.at(1)) * GameState->PixelsPerMeter,
                v2(p3.at(0), p3.at(1)) * GameState->PixelsPerMeter, rgb_to_imgui(color));
        },
        py::arg("p1"), py::arg("p2"), py::arg("p3"), py::arg("color"));

    m.def(
        "circle",
        [](py::array_t<f64> center, f64 radius, u32 color, s32 numSegments, f64 thickness) {
            assert(GameState && "State not initialized");
            assert(center.request().size == 2 && "rect requires p1 to be an array of size 2");
            GameState->ViewportDrawlist->AddCircle(v2(center.at(0), center.at(1)) * GameState->PixelsPerMeter,
                                                   (f32) radius * GameState->PixelsPerMeter, rgb_to_imgui(color),
                                                   numSegments, (f32) thickness);
        },
        py::arg("center"), py::arg("radius"), py::arg("color"), py::arg("num_segments") = 12,
        py::arg("thickness") = 1.0);

    m.def(
        "circle_filled",
        [](py::array_t<f64> center, f64 radius, u32 color, s32 numSegments) {
            assert(GameState && "State not initialized");
            assert(center.request().size == 2 && "rect requires p1 to be an array of size 2");
            GameState->ViewportDrawlist->AddCircleFilled(v2(center.at(0), center.at(1)) * GameState->PixelsPerMeter,
                                                         (f32) radius * GameState->PixelsPerMeter, rgb_to_imgui(color),
                                                         numSegments);
        },
        py::arg("center"), py::arg("radius"), py::arg("color"), py::arg("num_segments") = 12);
}

/*
    IMGUI_API void AddText(const ImVec2 &pos, ImU32 col, const char *text_begin, const char *text_end = NULL);
    IMGUI_API void AddText(const ImFont *font, float font_size, const ImVec2 &pos, ImU32 col, const char *text_begin,
                           const char *text_end = NULL, float wrap_width = 0.0f,
                           const ImVec4 *cpu_fine_clip_rect = NULL);
    IMGUI_API void AddPolyline(const ImVec2 *points, int num_points, ImU32 col, bool closed, float thickness);
    IMGUI_API void AddConvexPolyFilled(
        const ImVec2 *points, int num_points,
        ImU32 col);  // Note: Anti-aliased filling requires points to be in clockwise order.
    IMGUI_API void AddBezierCurve(const ImVec2 &pos0, const ImVec2 &cp0, const ImVec2 &cp1, const ImVec2 &pos1,
                                  ImU32 col, float thickness, int num_segments = 0);

    // Image primitives
    // - Read FAQ to understand what ImTextureID is.
    // - "p_min" and "p_max" represent the upper-left and lower-right corners of the rectangle.
    // - "uv_min" and "uv_max" represent the normalized texture coordinates to use for those corners. Using (0,0)->(1,1)
    // texture coordinates will generally display the entire texture.
    IMGUI_API void AddImage(ImTextureID user_texture_id, const ImVec2 &p_min, const ImVec2 &p_max,
                            const ImVec2 &uv_min = ImVec2(0, 0), const ImVec2 &uv_max = ImVec2(1, 1),
                            ImU32 col = IM_COL32_WHITE);
    IMGUI_API void AddImageQuad(ImTextureID user_texture_id, const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3,
                                const ImVec2 &p4, const ImVec2 &uv1 = ImVec2(0, 0), const ImVec2 &uv2 = ImVec2(1, 0),
                                const ImVec2 &uv3 = ImVec2(1, 1), const ImVec2 &uv4 = ImVec2(0, 1),
                                ImU32 col = IM_COL32_WHITE);
    IMGUI_API void AddImageRounded(ImTextureID user_texture_id, const ImVec2 &p_min, const ImVec2 &p_max,
                                   const ImVec2 &uv_min, const ImVec2 &uv_max, ImU32 col, float rounding,
                                   ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All);

    // Stateful path API, add points then finish with PathFillConvex() or PathStroke()
    inline void PathClear() { _Path.Size = 0; }
    inline void PathLineTo(const ImVec2 &pos) { _Path.push_back(pos); }
    inline void PathLineToMergeDuplicate(const ImVec2 &pos) {
        if (_Path.Size == 0 || memcmp(&_Path.Data[_Path.Size - 1], &pos, 8) != 0) _Path.push_back(pos);
    }
    inline void PathFillConvex(ImU32 col) {
        AddConvexPolyFilled(_Path.Data, _Path.Size, col);
        _Path.Size = 0;
    }  // Note: Anti-aliased filling requires points to be in clockwise order.
    inline void PathStroke(ImU32 col, bool closed, float thickness = 1.0f) {
        AddPolyline(_Path.Data, _Path.Size, col, closed, thickness);
        _Path.Size = 0;
    }
    IMGUI_API void PathArcTo(const ImVec2 &center, float radius, float a_min, float a_max, int num_segments = 10);
    IMGUI_API void PathArcToFast(const ImVec2 &center, float radius, int a_min_of_12,
                                 int a_max_of_12);  // Use precomputed angles for a 12 steps circle
    IMGUI_API void PathBezierCurveTo(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, int num_segments = 0);
    IMGUI_API void PathRect(const ImVec2 &rect_min, const ImVec2 &rect_max, float rounding = 0.0f,
                            ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All);

    // Advanced
    IMGUI_API void AddCallback(ImDrawCallback callback,
                               void *callback_data);  // Your rendering function must check for 'UserCallback' in
                                                      // ImDrawCmd and call the function instead of rendering triangles.
    IMGUI_API void
    AddDrawCmd();  // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering
                   // / blending). Otherwise primitives are merged into the same draw-call as much as possible
    IMGUI_API ImDrawList *CloneOutput() const;  // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer.

    // Advanced: Channels
    // - Use to split render into layers. By switching channels to can render out-of-order (e.g. submit foreground
    // primitives before background primitives)
    // - Use to minimize draw calls (e.g. if going back-and-forth between multiple non-overlapping clipping rectangles,
    // prefer to append into separate channels then merge at the end)
    inline void ChannelsSplit(int count) { _Splitter.Split(this, count); }
    inline void ChannelsMerge() { _Splitter.Merge(this); }
    inline void ChannelsSetCurrent(int n) { _Splitter.SetCurrentChannel(this, n); }
*/