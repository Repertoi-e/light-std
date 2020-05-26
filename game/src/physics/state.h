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

    f32 ZoomMin, ZoomMax;

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

    string PyCurrentDemo;
    array<string> PyDemoFiles;

    bool PyLoaded = false;
    py::module PyModule;
    py::function PyFrame;

// We need these in python.pyd
// @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack
#if defined DEBUG_MEMORY

    allocation_header *DEBUG_Head;
    thread::mutex *DEBUG_Mutex;
#endif
    u64 AllocationCount;
    game_memory *Memory;
    // End of dirty hack

    bool ShowOverlay = true;
    s32 OverlayCorner = 3;
};

inline game_state *GameState = null;

inline bucket_array<shader> *Shaders = null;
inline bucket_array<texture_2D> *Texture2Ds = null;

void reload_global_state();

void load_python_demo(string fileName);
void refresh_python_demo_files();

void report_python_error(py::error_already_set &e);

void editor_main();
void editor_scene_properties();

void viewport_render();
