#pragma once

#include <game.h>

enum class camera_type : s32 { Maya, FPS };

struct game_state {
    camera_type CameraType = camera_type::Maya;

    texture_2D ViewportRenderTarget;
    size_t FBSizeCBID = npos;

    vec4 ClearColor = {0.2f, 0.3f, 0.8f, 1.0f};

    bool NoGUI = false;
    bool Metrics = false;

    bool Overlay = true;
    s32 OverlayCorner = 3;

    bool MouseGrabbed = false;
};

struct camera {
    vec3 Position;
    vec3 Rotation;
    vec3 FocalPoint;
    f32 Pitch, Yaw;

    f32 Distance;
    f32 PanSpeed;
    f32 RotationSpeed;
    f32 ZoomSpeed;

    f32 MouseSensitivity;
    f32 Speed, SprintSpeed;

    camera();

    void reinit();
    void reset_constants();

    void update();
};

inline game_state *State = null;

void update_and_render_scene();
