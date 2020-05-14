// #include <dxmath/Inc/DirectXMath.h>
// using namespace DirectX;

#include "state.h"

static void release_scene() {
    Scene->~scene();
}

void reload_scene() {
    release_scene();
}

void update_and_render_scene() {
    auto *g = Graphics;
    auto *cam = &Scene->Camera;

    cam->update();

    if (GameMemory->MainWindow->is_visible() && GameState->Editor) {
        editor_scene_properties(cam);
    }
}
