#pragma once

#include <game.h>

struct game_state {
    texture_2D ViewportRenderTarget;
    size_t FBSizeCBID = npos, FocusCBID = npos;

    vec4 ClearColor = {0.2f, 0.3f, 0.8f, 1.0f};

    bool Editor = true;
    bool ShowMetrics = false;

    bool ShowOverlay = true;
    s32 OverlayCorner = 3;

    bool MouseGrabbed = false;
};

struct viewport {
    vec2 Position;
    f32 Rotation;
    vec2 Scale;
    
    f32 PanSpeed;
    f32 RotationSpeed;
    f32 ZoomSpeed;

    viewport();

    void reinit();
    void reset_constants();

    void update();
};

struct vertex {
    vec3 Position;
    vec4 Color;
};

// Uploaded to the GPU
struct entity_uniforms {
    mat4 ModelMatrix;
};

struct model : asset {
    buffer VB, IB;
    primitive_topology PrimitiveTopology;
};
inline catalog<model> *Models;

struct mesh {
    shader *Shader = null;
    model *Model = null;
};

struct entity {
    vec3 Position = vec3(0, 0, 0);
    quat Orientation = quat(0, 0, 0, 1);

    mesh Mesh;
};

// Uploaded to the GPU
struct scene_uniforms {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

struct scene {
    viewport Viewport;

    shader SceneShader;
    buffer SceneUB, EntityUB;

    bool GridFollowCamera = true;
    f32 GridSpacing = 1.0f;
    vec2i GridSize = {20, 20};

    scene_uniforms Uniforms;

    array<entity> Entities;

    size_t FBSizeCBID = npos;

    scene() { Viewport.reinit(); }  // Only runs once
};

inline scene *Scene = null;
inline game_state *State = null;

void update_and_render_scene();

void editor_main();
void editor_viewport_properties(viewport *vp);
void editor_assets();

// _p_ represents the center of the cuboid and _s_ is the radius in all axis, _c_ is a list of colors for each vertex
void generate_cuboid_model(model *m, vec3 p, vec3 s, vec4 c[8]);

void generate_grid_model(model *m, vec2i gridSize, f32 gridSpacing);
