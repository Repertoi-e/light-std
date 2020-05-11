#pragma once

#include <game.h>

struct game_state {
    texture_2D ViewportRenderTarget;
    size_t FBSizeCBID = npos, FocusCBID = npos;

    v4 ClearColor = {0.2f, 0.3f, 0.8f, 1.0f};

    bool Editor = true;
    bool ShowMetrics = false;

    bool ShowOverlay = true;
    s32 OverlayCorner = 3;

    bool MouseGrabbed = false;
};

struct alignas(16) camera {
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

    enum type : s32 { Maya, FPS };
    type Type = Maya;

    camera();

    void reinit();
    void reset_constants();

    void update();
};

void reload_scene();
void update_and_render_scene();

// @Volatile :vertex
struct vertex {
    v3 Position = {no_init};
    v4 Color = {no_init};
};

// Uploaded to the GPU
struct entity_uniforms {
    m44 ModelMatrix = {no_init};
};

struct model : asset {
    buffer VB, IB;
    primitive_topology PrimitiveTopology;
};

struct mesh {
    shader *Shader = null;
    model *Model = null;
};

struct alignas(16) entity {
    v3 Position = zero();
    quat Orientation = identity();

    mesh Mesh;

    bool Active = false;
    entity *Parent = null;
};

// Uploaded to the GPU
struct scene_uniforms {
    m44 ViewMatrix = {no_init};
    m44 ProjectionMatrix = {no_init};
};

struct alignas(16) scene {
    camera Camera;

    shader SceneShader;
    buffer SceneUB, EntityUB;

    entity *Grid = null;
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
inline asset_collection<texture_2D> *Texture2Ds = null;

entity *new_entity();

void reload_global_state();

void editor_main();
void editor_scene_properties(camera *cam);
void editor_assets();

// _p_ represents the center of the cuboid and _s_ is the radius in all axis, _c_ is a list of colors for each vertex
void generate_cuboid_model(model *m, v3 p, v3 s, v4 c[8]);
void generate_grid_model(model *m, vec2<s32> gridSize, f32 gridSpacing);
