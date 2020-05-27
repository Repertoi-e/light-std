#include "state.h"

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
            GameState->ViewportDrawlist->AddLine(v2(p1.at(0), p1.at(1)), v2(p2.at(0), p2.at(1)), color,
                                                 (f32) thickness);
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
            GameState->ViewportDrawlist->AddRect(v2(p1.at(0), p1.at(1)), v2(p2.at(0), p2.at(1)), color, (f32) rounding,
                                                 cornerFlags, (f32) thickness);
        },
        py::arg("p1"), py::arg("p2"), py::arg("color"), py::arg("rounding") = 0.0,
        py::arg("cornerFlags") = ImDrawCornerFlags_None, py::arg("thickness") = 1.0);
}

/*
    IMGUI_API void AddRectFilled(const ImVec2 &p_min, const ImVec2 &p_max, ImU32 col, float rounding = 0.0f,
                                 ImDrawCornerFlags rounding_corners =
                                     ImDrawCornerFlags_All);  // a: upper-left, b: lower-right (== upper-left + size)
    IMGUI_API void AddRectFilledMultiColor(const ImVec2 &p_min, const ImVec2 &p_max, ImU32 col_upr_left,
                                           ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left);
    IMGUI_API void AddQuad(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &p4, ImU32 col,
                           float thickness = 1.0f);
    IMGUI_API void AddQuadFilled(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &p4, ImU32 col);
    IMGUI_API void AddTriangle(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, ImU32 col, float thickness = 1.0f);
    IMGUI_API void AddTriangleFilled(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, ImU32 col);
    IMGUI_API void AddCircle(const ImVec2 &center, float radius, ImU32 col, int num_segments = 12,
                             float thickness = 1.0f);
    IMGUI_API void AddCircleFilled(const ImVec2 &center, float radius, ImU32 col, int num_segments = 12);
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