#if !defined LE_BUILDING_GAME
#error Error
#endif

#include "state.h"

viewport::viewport() {
    reinit();
    reset_constants();
}

void viewport::reinit() {
    Position = vec2(0, 0);
    Rotation = 0.0f;
}

void viewport::reset_constants() {
    PanSpeed = 0.0015f;
    RotationSpeed = 0.002f;
    ZoomSpeed = 0.2f;
}

void viewport::update() {
    // The viewport window may not be in an additional imgui window since we don't allow moving it,
    // so assuming it's in the main window's viewport is fine.
    auto *win = GameMemory->MainWindow;
}
