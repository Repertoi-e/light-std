#pragma once

#include <game.h>

struct game_state {
    texture_2D ViewportTexture;
    vec4 ClearColor = {0.2f, 0.3f, 0.8f, 1.0f};

    bool NoGUI = false;
    bool Metrics = false;

    bool Overlay = true;
    s32 OverlayCorner = 3;

    bool MouseGrabbed = false;

    size_t FBSizeCBID = npos;
};

struct camera {
    vec3 Position = vec3(0.0f, 25.0f, -25.0f);
    vec3 Rotation = vec3(90.0f, 0.0f, 0.0f);

    f32 MouseSensitivity = 0.002f;
    f32 Speed = 0.2f, SprintSpeed = Speed * 4;
    f32 Pitch = 0.7f, Yaw = 2.4f;

    void update();
};

inline game_state *State = null;

void update_and_render_scene();
