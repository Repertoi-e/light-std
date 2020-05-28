#include "state.h"
#include "vendor/pybind11/pybind11.h"

void viewport_render() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport", null, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav);
    ImGui::PopStyleVar(1);

    v2 viewportPos = ImGui::GetWindowPos();
    v2 viewportSize = ImGui::GetWindowSize();
    GameState->ViewportPos = viewportPos;
    GameState->ViewportSize = viewportSize;
    {
        auto *d = GameState->ViewportDrawlist = ImGui::GetWindowDrawList();

        // Add a colored rectangle which serves as a background
        d->AddRectFilled(viewportPos, viewportPos + viewportSize,
                         ImGui::ColorConvertFloat4ToU32(GameState->ClearColor));

        auto startVertex = d->VtxBuffer.Size;
        //
        // Here we render everything from Python side
        //
        {
            if (GameState->PyLoaded && GameState->PyFrame && !GameMemory->RequestReloadNextFrame) {
                try {
                    GameState->PyFrame(GameMemory->FrameDelta);
                } catch (py::error_already_set &e) {
                    report_python_error(e);
                }
            }
        }
        auto endVertex = d->VtxBuffer.Size;

        // We now build our view transformation matrix
        auto *cam = &GameState->Camera;

        // We scale and rotate based on the screen center
        m33 t = translation(viewportSize / 2 + cam->Position);
        m33 it = inverse(t);

        auto scaleRotate = dot(it, (m33) scale(cam->Scale));
        scaleRotate = dot(scaleRotate, (m33) rotation_z(-cam->Roll));
        scaleRotate = dot(scaleRotate, t);

        // Move origin to the top left of the viewport, by default it's in the top left of the whole application window.
        auto translate = dot((m33) translation(viewportPos), (m33) translation(-cam->Position));

        GameState->ViewMatrix = dot(scaleRotate, translate);
        GameState->InverseViewMatrix = inverse(GameState->ViewMatrix);

        auto *p = d->VtxBuffer.Data + startVertex;
        For(range(startVertex, endVertex)) {
            p->pos = dot((v2) p->pos, GameState->ViewMatrix);  // Apply the view matrix to each vertex
            ++p;
        }
    }
    ImGui::End();
}
