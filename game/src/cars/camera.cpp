#include "state.h"

void camera::update() {
    // @TODO The viewport window may be in an additional imgui window, and we don't handle that yet!
    auto *win = GameMemory->MainWindow;

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
