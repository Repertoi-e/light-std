#pragma once

#include <game.h>

#include "vendor/pybind11/eval.h"
#include "vendor/pybind11/numpy.h"
#include "vendor/pybind11/pybind11.h"

namespace py = pybind11;

struct camera {
    v2 Position = {no_init};
    v2 Scale = {no_init};
    f32 Roll;

    f32 PanSpeed;
    f32 RotationSpeed;
    f32 ZoomSpeed;

    f32 ZoomMin, ZoomMax, ZoomSpeedup;

    camera() { reinit(); }

    void reinit();
    void reset_constants();

    void update();
};

struct game_state {
    v4 ClearColor = {0.0f, 0.017f, 0.099f, 1.0f};

    camera Camera;
    m33 ViewMatrix = {no_init};
    ImDrawList *ViewportDrawlist;
    v2 ViewportPos = {no_init};
    v2 ViewportSize = {no_init};

    bool ShowOverlay = true;
    s32 OverlayCorner = 3;

    bool MouseGrabbed = false;
};

struct script {
    file::path FilePath;
    string Contents;
    py::module Module;
};

inline game_state *GameState = null;

inline asset_collection<shader> *Shaders = null;
inline asset_collection<texture_2D> *Texture2Ds = null;

void reload_global_state();

void editor_main();
void editor_scene_properties();

void viewport_render();
