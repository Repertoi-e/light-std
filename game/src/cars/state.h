#pragma once

#include <game.h>

enum class camera_type : s32 { Maya, FPS };

struct game_state {
    camera_type CameraType = camera_type::Maya;

    texture_2D ViewportRenderTarget;
    size_t FBSizeCBID = npos, FocusCBID = npos;

    v4 ClearColor = {0.2f, 0.3f, 0.8f, 1.0f};

    bool Editor = true;
    bool ShowMetrics = false;

    bool ShowOverlay = true;
    s32 OverlayCorner = 3;

    bool MouseGrabbed = false;
};

struct camera {
    v3 Position = {no_init};
    v3 Rotation = {no_init};
    v3 FocalPoint = {no_init};
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

void reload_scene();
void update_and_render_scene();

struct vertex {
    v3 Position;
    v4 Color;
};

// Uploaded to the GPU
struct entity_uniforms {
    m44 ModelMatrix = identity();
};

struct model : asset {
    buffer VB, IB;
    primitive_topology PrimitiveTopology;
};

struct mesh {
    shader *Shader = null;
    model *Model = null;
};

struct entity {
    v3 Position = v3(0, 0, 0);
    quat Orientation = quat(0, 0, 0, 1);

    mesh Mesh;
};

// Uploaded to the GPU
struct scene_uniforms {
    m44 ViewMatrix = identity();
    m44 ProjectionMatrix = identity();
};

struct scene {
    camera Camera;

    shader SceneShader;
    buffer SceneUB, EntityUB;

    bool GridFollowCamera = true;
    f32 GridSpacing = 1.0f;
    vec2<s32> GridSize = {20, 20};

    scene_uniforms Uniforms;

    array<entity> Entities;

    size_t FBSizeCBID = npos;

    scene() { Camera.reinit(); }  // Only runs once
};

inline game_state *GameState = null;
inline scene *Scene = null;
inline asset_collection<model> *Models = null;
inline asset_collection<shader> *Shaders = null;
void reload_global_state();

void editor_main();
void editor_scene_properties(camera *cam);
void editor_assets();

// _p_ represents the center of the cuboid and _s_ is the radius in all axis, _c_ is a list of colors for each vertex
void generate_cuboid_model(model *m, v3 p, v3 s, v4 c[8]);
void generate_grid_model(model *m, vec2<s32> gridSize, f32 gridSpacing);
