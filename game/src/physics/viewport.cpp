#include "state.h"
#include "vendor/pybind11/pybind11.h"

void viewport_render() {
    ImGui::Begin("Viewport");
    {
        auto viewportPos = GameState->ViewportPos;
        auto viewportSize = GameState->ViewportSize;

        auto *d = GameState->ViewportDrawlist = ImGui::GetWindowDrawList();

        // Add a colored rectangle which serves as a background
        d->AddRectFilled(viewportPos, viewportPos + viewportSize,
                         ImGui::ColorConvertFloat4ToU32(GameState->ClearColor));

        //
        // Here we render everything
        //
        {
            v2 pos(50, 50);
            v2 size(20, 20);
            d->AddRectFilled(pos, pos + size, ImGui::ColorConvertFloat4ToU32(v4(1, 0, 1, 1)));
        }

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

        auto *p = d->VtxBuffer.Data + 4;  // Don't transform the first 4 vertices (the background rectangle)
        For(range(4, d->VtxBuffer.Size)) {
            p->pos = dot((v2) p->pos, GameState->ViewMatrix); // Apply the view matrix to each vertex
            ++p;
        }
    }
    ImGui::End();
}
