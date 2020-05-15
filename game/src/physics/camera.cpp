#include "state.h"

void camera::reinit() {
    Position = v2(0, 0);
    Roll = 0;
    Scale = v2(1, 1);
    reset_constants();
}

void camera::reset_constants() {
    PanSpeed = 0.1f;
    RotationSpeed = 0.003f;
    ZoomSpeed = 0.005f;
    ZoomMin = 0.1f;
    ZoomMax = 10.0f;
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

        if (win->MouseButtons[Mouse_Button_Middle]) {
            f32 speed = PanSpeed * ZoomMax / Scale.x;

            Position += -right * delta.x * speed;
            Position += -up * delta.y * speed;
        } else if (win->MouseButtons[Mouse_Button_Left]) {
            Roll += delta.x * RotationSpeed;
        } else if (win->MouseButtons[Mouse_Button_Right]) {
            // We map our scale range [ZoomMin, ZoomMax] to the range [1, ZoomSpeedup] 
            // and then we speedup our scaling by a cubic factor.
            // (Faster zooming the more zoomed you are.)
            f32 x = 1 + 1 / (ZoomMax - ZoomMin) * (Scale.x - ZoomMin);
            f32 speed = x * x * x;
            Scale += delta.y * ZoomSpeed * speed;

            if (Scale.x < ZoomMin) Scale = v2(ZoomMin, ZoomMin);
            if (Scale.x > ZoomMax) Scale = v2(ZoomMax, ZoomMax);
        }
    }
    orientation = rotation_rpy(0.0f, 0.0f, -Roll);
}
