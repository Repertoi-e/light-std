#include "state.h"

void viewport_render(ImDrawList *d, v2 windowSize) {
    
    v2 pos(0, 0);
    v2 size(20, 20);

    d->AddRectFilled(v2(0, 0), windowSize, ImGui::ColorConvertFloat4ToU32(GameState->ClearColor));
}

