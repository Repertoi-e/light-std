#include "state.h"

#include "vendor/pybind11/pybind11.h"

void viewport_render() {
    auto *d = GameState->ViewportDrawlist;
    auto viewportSize = GameState->ViewportSize;
    
    v2 pos(50, 50);
    v2 size(20, 20);
    d->AddRectFilled(pos, pos + size, ImGui::ColorConvertFloat4ToU32(v4(1, 0, 1, 1)));

    // d->AddLine(, 0, 1);
}

