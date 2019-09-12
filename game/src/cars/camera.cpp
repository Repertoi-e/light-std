#include "state.h"

camera::camera() {
    reinit();
    reset_constants();
}

void camera::reinit() {
    Position = vec3(-17.678f, 25.0f, -17.678f);
    Rotation = vec3(-45.0f, -135.0f, 0.0f);

    Yaw = 3.0f * PI / 4.0f;
    Pitch = PI / 4.0f;

    FocalPoint = vec3(0, 0, 0);
    Distance = Position.distance(FocalPoint);
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

    if (State->CameraType == camera_type::Maya) {
        static vec2i lastMouse = {0, 0};

        quat orientation = quat::ROTATION_Y(-Yaw) * quat::ROTATION_X(-Pitch);
        vec3 right = quat::ROTATE(orientation, vec3(1, 0, 0));
        vec3 up = quat::ROTATE(orientation, vec3(0, 1, 0));
        vec3 forward = quat::ROTATE(orientation, -vec3(0, 0, 1));

        if (win->Keys[Key_LeftControl]) {
            vec2i mouse = win->get_cursor_pos();
            vec2 delta = {(f32) mouse.x - lastMouse.x, (f32) mouse.y - lastMouse.y};
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

        orientation = quat::ROTATION_Y(-Yaw) * quat::ROTATION_X(-Pitch);
        Rotation = orientation.to_euler_angles() * (180.0f / PI);
    } else if (State->CameraType == camera_type::FPS) {
        if (State->MouseGrabbed) {
            vec2i windowSize = win->get_size();

            vec2i delta = win->get_cursor_pos() - windowSize / 2;
            Yaw += delta.x * MouseSensitivity;
            Pitch += delta.y * MouseSensitivity;
            win->set_cursor_pos(windowSize / 2);

            quat orientation = quat::ROTATION_Y(-Yaw) * quat::ROTATION_X(-Pitch);
            Rotation = orientation.to_euler_angles() * (180.0f / PI);

            vec3 right = quat::ROTATE(orientation, vec3(1, 0, 0));
            vec3 forward = quat::ROTATE(orientation, -vec3(0, 0, 1));
            vec3 up = vec3(0, 1, 0);

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
