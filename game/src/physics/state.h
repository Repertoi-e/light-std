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
};

void camera_reinit(camera *cam);
void camera_reset_constants(camera *cam);
void camera_update(camera *cam);

struct game_state {
    v4 ClearColor = {0.0f, 0.017f, 0.099f, 1.0f};

    camera Camera;

    m33 ViewMatrix = {no_init};
    m33 InverseViewMatrix = {no_init};

    ImDrawList *ViewportDrawlist;
    v2 ViewportPos = {no_init};
    v2 ViewportSize = {no_init};

    void (*ViewportAddLine)(v2 p1, v2 p2, u32 color, f32 thickness);
    void (*ViewportAddRect)(v2 p1, v2 p2, u32 color, f32 rounding, ImDrawCornerFlags_ cornerFlags, f32 thickness);
    void (*ViewportAddRectFilled)(v2 p1, v2 p2, u32 color, f32 rounding, ImDrawCornerFlags_ cornerFlags);
    void (*ViewportAddRectFilledMultiColor)(v2 p1, v2 p2, u32 color_ul, u32 color_ur, u32 color_dr, u32 color_dl);
    void (*ViewportAddQuad)(v2 p1, v2 p2, v2 p3, v2 p4, u32 color, f32 thickness);
    void (*ViewportAddQuadFilled)(v2 p1, v2 p2, v2 p3, v2 p4, u32 color);
    void (*ViewportAddTriangle)(v2 p1, v2 p2, v2 p3, u32 color, f32 thickness);
    void (*ViewportAddTriangleFilled)(v2 p1, v2 p2, v2 p3, u32 color);
    void (*ViewportAddCircle)(v2 center, f32 radius, u32 color, s32 numSegments, f32 thickness);
    void (*ViewportAddCircleFilled)(v2 center, f32 radius, u32 color, s32 numSegments);
    void (*ViewportAddConvexPolyFilled)(const f32 *data, s32 count, u32 color);

    bool EditorShowShapeType = false;
    bool EditorShowImpulseResolution = false;
    bool EditorShowContinuousCollision = false;
    bool EditorShowCalculateContactPoints = false;
    bool EditorShowShowContactPoints = false;

    s32 EditorShapeCircle = false;
    bool EditorImpulseResolution = false;
    bool EditorContinuousCollision = false;
    bool EditorCalculateContactPoints = false;
    bool EditorShowContactPoints = false;

    // We scale coordinates by this amount to appear better on the screen
    f32 PixelsPerMeter = 50;

    string PyCurrentDemo;
    array<string> PyDemoFiles;

    bool PyLoaded = false;
    py::module PyModule;
    py::function PyFrame, PyEditorVariable, PyMouseClick, PyMouseRelease, PyMouseMove;

// We need these in python.pyd
// @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack @Hack
#if defined DEBUG_MEMORY

    allocation_header *DEBUG_Head;
    thread::mutex *DEBUG_Mutex;
#endif
    u64 AllocationCount;
    game_memory *Memory;
};

void reload_global_state();

void load_python_demo(string fileName);
void refresh_python_demo_files();

void report_python_error(py::error_already_set &e);

void editor_main();
void editor_scene_properties();

void viewport_render();

// @TODO: #define global inline? (just for annotation)

inline game_state *GameState = null;
