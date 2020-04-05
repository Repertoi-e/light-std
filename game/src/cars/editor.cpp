#include "state.h"

void editor_main() {
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("CDock Window", null,
                 ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                     ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar(3);

    ImGuiID dockspaceID = ImGui::GetID("CDock");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f));

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Game")) {
            if (ImGui::MenuItem("VSync", "", GameMemory->MainWindow->Flags & window::VSYNC))
                GameMemory->MainWindow->Flags ^= window::VSYNC;
            if (ImGui::MenuItem("Editor", "Ctrl + F", GameState->Editor)) {
                GameState->Editor = !GameState->Editor;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Show overlay", "", GameState->ShowOverlay)) {
                GameState->ShowOverlay = !GameState->ShowOverlay;
            }
            if (ImGui::MenuItem("Show imgui metrics", "", GameState->ShowMetrics)) {
                GameState->ShowMetrics = !GameState->ShowMetrics;
            }
            ImGui::EndMenu();
        }
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("This is the editor view of the light-std game engine...");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        ImGui::EndMenuBar();
    }
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport", null, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav);
    ImGui::PopStyleVar(1);

    auto windowPos = ImGui::GetWindowPos();
    auto windowSize = ImGui::GetWindowSize();
    {
        auto *drawList = ImGui::GetWindowDrawList();

        f32 viewportRatio = (f32) GameState->ViewportRenderTarget.Width / GameState->ViewportRenderTarget.Height;
        f32 windowRatio = windowSize.x / windowSize.y;

        v2 renderableSize = windowSize;
        v2 offset = {no_init};
        if (viewportRatio < windowRatio) {
            renderableSize.x =
                (GameState->ViewportRenderTarget.Width * (windowSize.y / GameState->ViewportRenderTarget.Height));
            offset = v2{(windowSize.x - renderableSize.x) / 2, 0.0f};
        } else if (viewportRatio > windowRatio) {
            renderableSize.y =
                (GameState->ViewportRenderTarget.Height * (windowSize.x / GameState->ViewportRenderTarget.Width));
            offset = v2{0, (windowSize.y - renderableSize.y) / 2};
        }
        offset += v2(6 * viewportRatio, 6);
        renderableSize -= v2(18 * viewportRatio, 18);

        drawList->AddImage(&GameState->ViewportRenderTarget, (v2) windowPos + offset,
                           (v2) windowPos + offset + renderableSize);
        if (GameState->MouseGrabbed) {
            drawList->AddRect((v2) windowPos + offset, (v2) windowPos + offset + renderableSize, 0xffffffff);
        }
    }

    if (GameState->CameraType == camera_type::FPS && ImGui::InvisibleButton("##viewport", windowSize)) {
        GameState->MouseGrabbed = true;
        GameMemory->MainWindow->set_cursor_mode(window::CURSOR_DISABLED);
    }

    if (GameState->ShowOverlay) {
        if (GameState->OverlayCorner != -1) {
            ImVec2 pivot = ImVec2((GameState->OverlayCorner & 1) ? 1.0f : 0.0f, (GameState->OverlayCorner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(
                ImVec2((GameState->OverlayCorner & 1) ? (windowPos.x + windowSize.x - 25) : (windowPos.x + 10),
                       (GameState->OverlayCorner & 2) ? (windowPos.y + windowSize.y - 10) : (windowPos.y + 25)),
                ImGuiCond_Always, pivot);
        }

        ImGui::SetNextWindowBgAlpha(0.35f);
        if (ImGui::Begin("Overlay", &GameState->ShowOverlay,
                         (GameState->OverlayCorner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDocking |
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            if (ImGui::BeginPopupContextWindow()) {
                if (ImGui::MenuItem("Custom", null, GameState->OverlayCorner == -1)) GameState->OverlayCorner = -1;
                if (ImGui::MenuItem("Top-left", null, GameState->OverlayCorner == 0)) GameState->OverlayCorner = 0;
                if (ImGui::MenuItem("Top-right", null, GameState->OverlayCorner == 1)) GameState->OverlayCorner = 1;
                if (ImGui::MenuItem("Bottom-left", null, GameState->OverlayCorner == 2)) GameState->OverlayCorner = 2;
                if (ImGui::MenuItem("Bottom-right", null, GameState->OverlayCorner == 3)) GameState->OverlayCorner = 3;
                if (GameState->ShowOverlay && ImGui::MenuItem("Close")) GameState->ShowOverlay = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }
    ImGui::End();

    if (GameState->ShowMetrics) ImGui::ShowMetricsWindow(&GameState->ShowMetrics);
}

static void update_grid() {
    entity *grid = null;  // @Speed
    For_as(it_index, range(Scene->Entities.Count)) {
        auto *it = &Scene->Entities[it_index];
        if (it->Mesh.Model->Name == "Grid Model") {
            grid = it;
            break;
        }
    }
    assert(grid);

    grid->Mesh.Shader->bind();
    generate_grid_model(grid->Mesh.Model, Scene->GridSize, Scene->GridSpacing);
}

extern s32 ImFormatString(char *buf, size_t buf_size, const char *fmt, ...);

inline bool slider_float_with_steps(const char *label, float *v, float v_min, float v_max, float v_step,
                                    const char *display_format = "%.3f") {
    char text_buf[64] = {};
    ImFormatString(text_buf, IM_ARRAYSIZE(text_buf), display_format, *v);

    // Map from [v_min,v_max] to [0,N]
    const int countValues = int((v_max - v_min) / v_step);
    int v_i = int((*v - v_min) / v_step);
    const bool value_changed = ImGui::SliderInt(label, &v_i, 0, countValues, text_buf);

    // Remap from [0,N] to [v_min,v_max]
    *v = v_min + float(v_i) * v_step;
    return value_changed;
}

void editor_scene_properties(camera *cam) {
    ImGui::Begin("Scene Properties", null);
    ImGui::Text("Camera");
    ImGui::BeginChild("##camera", {0, 180}, true);
    {
        if (ImGui::RadioButton("Maya", (s32 *) &GameState->CameraType, (s32) camera_type::Maya)) cam->reinit();
        ImGui::SameLine();
        if (ImGui::RadioButton("FPS", (s32 *) &GameState->CameraType, (s32) camera_type::FPS)) cam->reinit();

        ImGui::Text("Position: %.3f, %.3f, %.3f", cam->Position.x, cam->Position.y, cam->Position.z);
        ImGui::Text("Rotation: %.3f, %.3f, %.3f", cam->Rotation.x, cam->Rotation.y, cam->Rotation.z);
        ImGui::Text("Pitch: %.3f, yaw: %.3f", cam->Pitch, cam->Yaw);

        if (GameState->CameraType == camera_type::Maya) {
            ImGui::PushItemWidth(-140);
            ImGui::SliderFloat("Pan speed", &cam->PanSpeed, 0.0005f, 0.005f);
            ImGui::PushItemWidth(-140);
            ImGui::SliderFloat("Rotation speed", &cam->RotationSpeed, 0.0005f, 0.005f);
            ImGui::PushItemWidth(-140);
            ImGui::SliderFloat("Zoom speed", &cam->ZoomSpeed, 0.05f, 0.5f);
        } else if (GameState->CameraType == camera_type::FPS) {
            ImGui::PushItemWidth(-140);
            ImGui::SliderFloat("Speed", &cam->Speed, 0.01f, 10);
            ImGui::PushItemWidth(-140);
            ImGui::SliderFloat("Sprint speed", &cam->SprintSpeed, 0.01f, 10);
            ImGui::PushItemWidth(-140);
            ImGui::SliderFloat("Mouse sensitivity", &cam->MouseSensitivity, 0.0001f, 0.01f);
        }
        if (ImGui::Button("Default camera constants")) cam->reset_constants();

        ImGui::EndChild();
    }

    ImGui::ColorPicker3("Clear color", &GameState->ClearColor.x);

    if (ImGui::Checkbox("Grid follow camera", &Scene->GridFollowCamera)) {
        if (!Scene->GridFollowCamera) {
            For_as(it_index, range(Scene->Entities.Count)) {  // @Speed
                auto *it = &Scene->Entities[it_index];
                if (it->Mesh.Model->Name == "Grid Model") {
                    it->Position.x = it->Position.z = 0;
                    break;
                }
            }
        }
    }
    if (slider_float_with_steps("Grid spacing", &Scene->GridSpacing, 0.5f, 10.0f, 0.5f)) update_grid();
    if (ImGui::SliderInt2("Grid size", &Scene->GridSize.x, 1, 50)) update_grid();
    ImGui::End();
}

void editor_assets() {
    ImGui::Begin("Assets", null);
    if (ImGui::TreeNode("Shaders")) {
        auto *b = Shaders->BucketList;
        while (b) {
            For(b->Assets) {
                auto *name = it->Name.to_c_string(Context.TemporaryAlloc);
                if (ImGui::TreeNode(name)) {
                    ImGui::Text("Name: %s", name);
                    ImGui::Text("File path: %s", it->FilePath.UnifiedPath.to_c_string(Context.TemporaryAlloc));
                    ImGui::TreePop();
                }
            }
            b = b->Next;
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Models")) {
        auto *b = Models->BucketList;
        while (b) {
            For(b->Assets) {
                auto *name = it->Name.to_c_string(Context.TemporaryAlloc);
                if (ImGui::TreeNode(name)) {
                    ImGui::Text("Name: %s", name);
                    ImGui::Text("File path: %s", it->FilePath.UnifiedPath.to_c_string(Context.TemporaryAlloc));
                    ImGui::TreePop();
                }
            }
            b = b->Next;
        }
        ImGui::TreePop();
    }
    ImGui::End();
}
