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
            ImGui::TextUnformatted("This is the editor view of the light-std graphics engine...");
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
        auto *d = ImGui::GetWindowDrawList();
        viewport_render(d, windowSize);

        auto viewMatrix =
            (m33) translation(windowPos.x, windowPos.y);  // Move origin to top left of viewport
                                                          // (by default its top left of the while window window)

        auto *cam = &Scene->Camera;
        viewMatrix = dot(viewMatrix, (m33) translation(-cam->Position));
        viewMatrix = dot(viewMatrix, (m33) rotation_z(-cam->Roll));
        viewMatrix = dot(viewMatrix, (m33) scale(cam->Scale));

        auto *p = d->VtxBuffer.Data;
        For(range(d->VtxBuffer.Size)) {
            p->pos = dot((v2) p->pos, viewMatrix);
            ++p;
        }
    }

    if (GameState->ShowOverlay) {
        if (GameState->OverlayCorner != -1) {
            ImVec2 pivot =
                ImVec2((GameState->OverlayCorner & 1) ? 1.0f : 0.0f, (GameState->OverlayCorner & 2) ? 1.0f : 0.0f);
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

void editor_scene_properties(camera *cam) {
    ImGui::Begin("Scene Properties", null);
    ImGui::Text("Camera");
    ImGui::BeginChild("##camera", {0, 180}, true);
    {
        ImGui::Text("Position: %.3f, %.3f", cam->Position.x, cam->Position.y);
        ImGui::Text("Roll: %.3f", cam->Roll);
        ImGui::Text("Scale (zoom): %.3f, %.3f", cam->Scale.x, cam->Scale.y);

        ImGui::PushItemWidth(-140);
        ImGui::SliderFloat("Pan speed", &cam->PanSpeed, 0.005f, 0.5f);
        ImGui::PushItemWidth(-140);
        ImGui::SliderFloat("Rotation speed", &cam->RotationSpeed, 0.00005f, 0.0005f);
        ImGui::PushItemWidth(-140);
        ImGui::SliderFloat("Zoom speed", &cam->ZoomSpeed, 0.005f, 0.05f);

        if (ImGui::Button("Default camera constants")) cam->reset_constants();
        if (ImGui::Button("Reset camera")) cam->reinit();

        ImGui::EndChild();
    }
    ImGui::ColorPicker3("Clear color", &GameState->ClearColor.x);
    ImGui::End();
}
