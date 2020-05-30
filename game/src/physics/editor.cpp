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
            ImGui::EndMenu();
        }
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("This is the editor view of the light-std game engine.");
            ImGui::TextUnformatted("");
            ImGui::TextUnformatted("* Here are the controls for the camera:");
            ImGui::TextUnformatted("      Ctrl + Left Mouse -> rotate");
            ImGui::TextUnformatted("      Ctrl + Middle Mouse -> pan");
            ImGui::TextUnformatted("      Ctrl + Right Mouse -> zoom");
            ImGui::TextUnformatted("");
            ImGui::TextUnformatted("This project is licensed under the MIT license.");
            ImGui::TextUnformatted("Source code: github.com/Repertoi-e/light-std/");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        ImGui::EndMenuBar();
    }
    ImGui::End();
}

void editor_scene_properties() {
    auto *cam = &GameState->Camera;

    ImGui::Begin("Python", null);
    {
        auto demoFiles = GameState->PyDemoFiles;

        ImGui::Text("Select demo file:");
        if (ImGui::BeginCombo("##combo", GameState->PyCurrentDemo.to_c_string(Context.TemporaryAlloc))) {
            For(demoFiles) {
                bool isSelected = GameState->PyCurrentDemo == it;
                if (ImGui::Selectable(it.to_c_string(Context.TemporaryAlloc), &isSelected)) {
                    if (GameState->PyCurrentDemo != it) {
                        GameState->PyCurrentDemo = it;
                        load_python_demo(it);
                    }
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Refresh demo files")) refresh_python_demo_files();
        ImGui::Text("");

        try {
            if (GameState->EditorShowShapeType) {
                ImGui::Text("Spawn shape (Right-Click):");
                if (ImGui::RadioButton("Polygon", &GameState->EditorShapeCircle, 0)) {
                    if (GameState->PyEditorVariable) GameState->PyEditorVariable("shape_spawn_type", "polygon");
                }
                if (ImGui::RadioButton("Circle", &GameState->EditorShapeCircle, 1)) {
                    if (GameState->PyEditorVariable) GameState->PyEditorVariable("shape_spawn_type", "circle");
                }
            }

            if (GameState->EditorShowImpulseResolution) {
                if (ImGui::Checkbox("Impulse based resolution", &GameState->EditorImpulseResolution)) {
                    if (GameState->PyEditorVariable) {
                        GameState->PyEditorVariable("impulse_resolution", GameState->EditorImpulseResolution);
                    }
                }
            }

            if (GameState->EditorShowContinuousCollision) {
                if (ImGui::Checkbox("Continuous collision detection", &GameState->EditorContinuousCollision)) {
                    if (GameState->PyEditorVariable) {
                        GameState->PyEditorVariable("continuous_collision", GameState->EditorContinuousCollision);
                    }
                }
            }
        } catch (py::error_already_set &e) {
            report_python_error(e);
        }
    }
    ImGui::End();

    ImGui::Begin("Camera", null);
    {
        if (ImGui::Button("Reset camera")) camera_reinit(cam);

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
        if (ImGui::Button("Default camera constants")) camera_reset_constants(cam);
    }
    ImGui::End();

    ImGui::Begin("Scene", null);
    ImGui::InputFloat("PPM", &GameState->PixelsPerMeter);
    ImGui::Text("Frame information:");
    ImGui::Text("  %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Clear color:");
    ImGui::ColorPicker3("", &GameState->ClearColor.x, ImGuiColorEditFlags_NoAlpha);
    if (ImGui::Button("Reset color")) GameState->ClearColor = {0.0f, 0.017f, 0.099f, 1.0f};
    ImGui::End();
}
