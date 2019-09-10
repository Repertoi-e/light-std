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

    s32 CameraType = 0;  // 0 - Maya, 1 - FPS

    size_t FBSizeCBID = npos;
};

struct camera {
    vec3 Position;
    vec3 Rotation;
    vec3 FocalPoint;
    f32 Pitch, Yaw;

    f32 Distance;
    f32 PanSpeed = 0.0015f;
    f32 RotationSpeed = 0.002f;
    f32 ZoomSpeed = 0.2f;

    f32 MouseSensitivity = 0.002f;
    f32 Speed = 0.2f, SprintSpeed = Speed * 4;

    void set_type(s32 type);
    void update();
};

inline game_state *State = null;

void update_and_render_scene();
