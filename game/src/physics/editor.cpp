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
            ImGui::Separator();
            if (ImGui::MenuItem("Show overlay", "", GameState->ShowOverlay)) {
                GameState->ShowOverlay = !GameState->ShowOverlay;
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

    v2 viewportPos = ImGui::GetWindowPos();
    v2 viewportSize = ImGui::GetWindowSize();
    GameState->ViewportPos = viewportPos;
    GameState->ViewportSize = viewportSize;
    {
        auto *d = GameState->ViewportDrawlist = ImGui::GetWindowDrawList();
        d->AddRectFilled(viewportPos, viewportPos + viewportSize,
                         ImGui::ColorConvertFloat4ToU32(GameState->ClearColor));

        viewport_render();

        auto *cam = &GameState->Camera;

        // We scale and rotate based on the screen center
        m33 t = translation(viewportSize / 2 + cam->Position);
        m33 it = inverse(t);

        auto scaleRotate = dot(it, (m33) scale(cam->Scale));
        scaleRotate = dot(scaleRotate, (m33) rotation_z(-cam->Roll));
        scaleRotate = dot(scaleRotate, t);

        // Move origin to top left of viewport, by default it's in the top left of the whole window.
        auto translate = dot((m33) translation(viewportPos), (m33) translation(-cam->Position));

        GameState->ViewMatrix = dot(scaleRotate, translate);

        auto *p = d->VtxBuffer.Data + 4;  // Don't transform the clear color rect
        For(range(4, d->VtxBuffer.Size)) {
            p->pos = dot((v2) p->pos, GameState->ViewMatrix);
            ++p;
        }
    }

    if (GameState->ShowOverlay) {
        if (GameState->OverlayCorner != -1) {
            ImVec2 pivot =
                ImVec2((GameState->OverlayCorner & 1) ? 1.0f : 0.0f, (GameState->OverlayCorner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(
                ImVec2((GameState->OverlayCorner & 1) ? (viewportPos.x + viewportSize.x - 25) : (viewportPos.x + 10),
                       (GameState->OverlayCorner & 2) ? (viewportPos.y + viewportSize.y - 10) : (viewportPos.y + 25)),
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
}

void editor_scene_properties() {
    auto *cam = &GameState->Camera;

    ImGui::Begin("View Properties", null);
    ImGui::Text("Camera");
    ImGui::BeginChild("##camera", {0, 247}, true);
    {
        if (ImGui::Button("Reset camera")) cam->reinit();

        ImGui::Text("Position: %.3f, %.3f", cam->Position.x, cam->Position.y);
        ImGui::Text("Roll: %.3f", cam->Roll);
        ImGui::Text("Scale (zoom): %.3f, %.3f", cam->Scale.x, cam->Scale.y);
        if (ImGui::Button("Reset rotation")) cam->Roll = 0;

        ImGui::PushItemWidth(-140);
        ImGui::InputFloat("Pan speed", &cam->PanSpeed);
        ImGui::PushItemWidth(-140);
        ImGui::InputFloat("Rotation speed", &cam->RotationSpeed);
        ImGui::PushItemWidth(-140);
        ImGui::InputFloat("Zoom speed", &cam->ZoomSpeed);

        ImGui::InputFloat2("Zoom min/max", &cam->ZoomMin);
        ImGui::InputFloat("Zoom speedup", &cam->ZoomSpeedup);

        if (ImGui::Button("Default camera constants")) cam->reset_constants();

        ImGui::EndChild();
    }
    ImGui::ColorPicker3("Clear color", &GameState->ClearColor.x, ImGuiColorEditFlags_NoAlpha);
    if (ImGui::Button("Reset color")) GameState->ClearColor = {0.0f, 0.017f, 0.099f, 1.0f};
    ImGui::End();
}
