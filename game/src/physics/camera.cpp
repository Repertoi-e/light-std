#include "state.h"

void camera_reinit(camera *cam) {
    cam->Position = v2(0, 0);
    cam->Roll = 0;
    cam->Scale = v2(1, 1);
    camera_reset_constants(cam);
}

void camera_reset_constants(camera *cam) {
    cam->PanSpeed = 0.1f;
    cam->RotationSpeed = 0.003f;
    cam->ZoomSpeed = 0.005f;
    cam->ZoomMin = 0.1f;
    cam->ZoomMax = 10.0f;
}

void camera_update(camera *cam) {
    // The viewport window may not be in an additional imgui window since
    // we don't allow moving it, so assuming this is is fine.
    auto *win = GameMemory->MainWindow;

    static vec2<s32> lastMouse = zero();

    f32 S = sin(cam->Roll);
    f32 C = cos(cam->Roll);

    v2 up(-S, C);
    v2 right(C, S);

    if (win->Keys[Key_LeftControl]) {
        vec2<s32> mouse = win->get_cursor_pos();
        v2 delta = {(f32) mouse.x - lastMouse.x, (f32) mouse.y - lastMouse.y};
        lastMouse = mouse;

        if (win->MouseButtons[Mouse_Button_Middle]) {
            f32 speed = cam->PanSpeed * cam->ZoomMax / cam->Scale.x;

            cam->Position += -right * delta.x * speed;
            cam->Position += -up * delta.y * speed;
        } else if (win->MouseButtons[Mouse_Button_Left]) {
            cam->Roll += delta.x * cam->RotationSpeed;
        } else if (win->MouseButtons[Mouse_Button_Right]) {
            // We map our scale range [ZoomMin, ZoomMax] to the range [1, ZoomSpeedup]
            // and then we speedup our scaling by a cubic factor.
            // (Faster zooming the more zoomed you are.)
            f32 x = 1 + 1 / (cam->ZoomMax - cam->ZoomMin) * (cam->Scale.x - cam->ZoomMin);
            f32 speed = x * x * x;
            cam->Scale += delta.y * cam->ZoomSpeed * speed;

            if (cam->Scale.x < cam->ZoomMin) cam->Scale = v2(cam->ZoomMin, cam->ZoomMin);
            if (cam->Scale.x > cam->ZoomMax) cam->Scale = v2(cam->ZoomMax, cam->ZoomMax);
        }
    }
}
