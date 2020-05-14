#include "state.h"

camera::camera() { reinit(); }

void camera::reinit() {
    Position = v2(0, 0);
    Roll = 0;
    Scale = v2(1, 1);
    reset_constants();
}

void camera::reset_constants() {
    PanSpeed = 0.0015f;
    RotationSpeed = 0.002f;
    ZoomSpeed = 0.2f;
}

void camera::update() {
    // The viewport window may not be in an additional imgui window since
    // we don't allow moving it, so assuming this is is fine.
    auto *win = GameMemory->MainWindow;

    static vec2<s32> lastMouse = zero();

    quat orientation = rotation_rpy(0.0f, 0.0f, -Roll);
    v2 up = rotate_vec(v3(0, 1, 0), orientation).xy;
    v2 right = rotate_vec(v3(1, 0, 0), orientation).xy;

    if (win->Keys[Key_LeftControl]) {
        vec2<s32> mouse = win->get_cursor_pos();
        v2 delta = {(f32) mouse.x - lastMouse.x, (f32) mouse.y - lastMouse.y};
        lastMouse = mouse;

        f32 maxDelta;
        if (abs(delta.x) > abs(delta.y)) {
            maxDelta = delta.x;
        } else {
            maxDelta = delta.y;
        }

        if (win->MouseButtons[Mouse_Button_Middle]) {
            Position += -right * delta.x * PanSpeed;
            Position += up * delta.y * PanSpeed;
        } else if (win->MouseButtons[Mouse_Button_Left]) {
            Roll += maxDelta * RotationSpeed;
        } else if (win->MouseButtons[Mouse_Button_Right]) {
            Scale += maxDelta * ZoomSpeed;
        }
    }
    orientation = rotation_rpy(0.0f, 0.0f, -Roll);
}
