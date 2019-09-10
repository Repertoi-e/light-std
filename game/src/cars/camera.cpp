#include "state.h"

void camera::set_type(s32 type) {
    Position = vec3(0.0f, 25.0f, -25.0f);
    Rotation = vec3(90.0f, 0.0f, 0.0f);

    if (type == 0) {
        FocalPoint = vec3(0, 0, 0);
        Distance = Position.distance(FocalPoint);

        Yaw = 3.0f * PI / 4.0f;
        Pitch = PI / 4.0f;
    } else {
        Pitch = 0.7f;
        Yaw = 2.4f;
    }
}

void camera::update() {
    // @TODO The viewport window may be in an additional imgui window, and we don't handle that yet!
    auto *win = GameMemory->MainWindow;

    if (State->CameraType == 0) {
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
    } else {
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
    }
}
