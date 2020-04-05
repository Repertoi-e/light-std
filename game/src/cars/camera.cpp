#include "state.h"

camera::camera() {
    reinit();
    reset_constants();
}

void camera::reinit() {
    Position = v3(-17.678f, 25.0f, -17.678f);
    Rotation = v3(-45.0f, -135.0f, 0.0f);

    Yaw = 3.0f * PI / 4.0f;
    Pitch = PI / 4.0f;

    FocalPoint = v3(0, 0, 0);
    Distance = len(FocalPoint - Position);
}

void camera::reset_constants() {
    PanSpeed = 0.0015f;
    RotationSpeed = 0.002f;
    ZoomSpeed = 0.2f;

    MouseSensitivity = 0.002f;
    Speed = 0.2f;
    SprintSpeed = Speed * 4;
}

void camera::update() {
    // The viewport window may not be in an additional imgui window since we don't allow moving it,
    // so assuming it's in the main window's viewport is fine.
    auto *win = GameMemory->MainWindow;

    if (GameState->CameraType == camera_type::Maya) {
        static vec2<s32> lastMouse = zero();

        quat orientation = rotation_rpy(0.0f, -Pitch, -Yaw);
        v3 up = rotate_vec(v3(0, 1, 0), orientation);
        v3 right = rotate_vec(v3(1, 0, 0), orientation);
        v3 forward = rotate_vec(v3(0, 0, -1), orientation);

        if (win->Keys[Key_LeftControl]) {
            vec2<s32> mouse = win->get_cursor_pos();
            v2 delta = {(f32) mouse.x - lastMouse.x, (f32) mouse.y - lastMouse.y};
            lastMouse = mouse;

            if (win->MouseButtons[Mouse_Button_Middle]) {
                FocalPoint += -right * delta.x * PanSpeed * Distance;
                FocalPoint += up * delta.y * PanSpeed * Distance;
            } else if (win->MouseButtons[Mouse_Button_Left]) {
                f32 yawSign = up.y < 0 ? -1.0f : 1.0f;
                Yaw += yawSign * delta.x * RotationSpeed;
                Pitch += delta.y * RotationSpeed;
            } else if (win->MouseButtons[Mouse_Button_Right]) {
                Distance -= delta.y * ZoomSpeed;
                if (Distance < 1.0f) {
                    FocalPoint += forward;
                    Distance = 1.0f;
                }
            }
        }
        Position = FocalPoint - forward * Distance;

        orientation = rotation_rpy(0.0f, -Pitch, -Yaw);
        Rotation = to_euler_angles(orientation) / TAU * 360;
    } else if (GameState->CameraType == camera_type::FPS) {
        if (GameState->MouseGrabbed) {
            vec2<s32> windowSize = win->get_size();
            vec2<s32> delta = win->get_cursor_pos() - windowSize / 2;
            Yaw += delta.x * MouseSensitivity;
            Pitch += delta.y * MouseSensitivity;
            win->set_cursor_pos(windowSize / 2);

            quat orientation = rotation_rpy(-Pitch, -Yaw, 0.0f);
            Rotation = to_euler_angles(orientation) / TAU * 360;

            v3 up = v3(0, 1, 0);
            v3 right = rotate_vec(v3(1, 0, 0), orientation);
            v3 forward = rotate_vec(v3(0, 0, -1), orientation);

            f32 speed = win->Keys[Key_LeftShift] ? SprintSpeed : Speed;

            if (win->Keys[Key_W]) {
                Position += forward * speed;
            } else if (win->Keys[Key_S]) {
                Position -= forward * speed;
            }
            if (win->Keys[Key_A]) {
                Position -= right * speed;
            } else if (win->Keys[Key_D]) {
                Position += right * speed;
            }

            if (win->Keys[Key_Space]) {
                Position += up * speed;
            }
            if (win->Keys[Key_LeftControl]) {
                Position -= up * speed;
            }
        }
    } else {
        assert(false);
    }
}
