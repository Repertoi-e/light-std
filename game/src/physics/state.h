#pragma once

#include <game.h>

struct game_state {
    v4 ClearColor = {0.2f, 0.3f, 0.8f, 1.0f};

    bool Editor = true;
    bool ShowMetrics = false;

    bool ShowOverlay = true;
    s32 OverlayCorner = 3;

    bool MouseGrabbed = false;
};

struct alignas(16) camera {
    v2 Position = {no_init};
    v2 Scale = {no_init};
    f32 Roll;

    f32 PanSpeed;
    f32 RotationSpeed;
    f32 ZoomSpeed;

    camera();

    void reinit();
    void reset_constants();

    void update();
};

void reload_scene();
void update_and_render_scene();

// Uploaded to the GPU
struct scene_uniforms {
    m44 ViewMatrix = {no_init};
    m44 ProjectionMatrix = {no_init};
};

struct alignas(16) scene {
    camera Camera;
    
    scene_uniforms Uniforms;

    scene() { Camera.reinit(); }  // Only runs once
};

inline game_state *GameState = null;
inline scene *Scene = null;

inline asset_collection<shader> *Shaders = null;
inline asset_collection<texture_2D> *Texture2Ds = null;

void reload_global_state();

void editor_main();
void editor_scene_properties(camera *cam);

void viewport_render(ImDrawList *d, v2 windowSize);
