#include "state.h"

void reload_global_state() {
    Context.AllocAlignment = 16;  // For SIMD

    MANAGE_GLOBAL_STATE(GameState);
    MANAGE_GLOBAL_STATE(AssetCatalog);

    // We need these in python.pyb
    GameState->Memory = GameMemory;
#if defined DEBUG_MEMORY
    GameState->DEBUG_Head = allocator::DEBUG_Head;
    GameState->DEBUG_Mutex = &allocator::DEBUG_Mutex;
#endif
    GameState->AllocationCount = allocator::AllocationCount;

    AssetCatalog->ensure_initted(file::path("data/"));

    //
    // This is very specific but still better than nothing...
    // The problem is that Py_Initialize doesn't find "encodings" module even though we have python in our path.
    // Setting PYTHONPATH and PYTHONHOME seems to fix the problem.
    //
    string path;
    if (os_get_env(&path, "PATH")) {
        if (path.Length) {
            array<string> paths;

            size_t semicolon, beginning = 0;
            while ((semicolon = path.find(";", beginning)) != -1) {
                // @Bug Big one... we can't store an array of substrings because when we resize the container and
                // move the elements, old ones attempt to get freed.
                paths.append(path.substring(beginning, semicolon));
                beginning += semicolon - beginning + 1;
            }

            string found;
            For(paths) {
                if (it.ends_with("ProgramData\\Anaconda3")) found = it;
                if (it.ends_with("ProgramData/Anaconda3")) found = it;
            }
            os_set_env("PYTHONPATH", found);
            os_set_env("PYTHONHOME", found);
        }
    }

    GameState->PyLoaded = false;

    void load_imgui_bindings_for_python();
    load_imgui_bindings_for_python();

    refresh_python_demo_files();
    if (GameState->PyCurrentDemo) {
        load_python_demo(GameState->PyCurrentDemo);
    } else {
        if (GameState->PyDemoFiles.Count) load_python_demo("demo_grabbing.py");
    }
}

void load_python_demo(string demo) {
    clone(&GameState->PyCurrentDemo, demo);

    if (GameState->PyLoaded) {
        if (GameState->PyModule) {
            if (py::hasattr(GameState->PyModule, "unload")) GameState->PyModule.attr("unload")();
        }
        GameMemory->RequestReloadNextFrame = true;
        return;
    }

    Py_Initialize();

    PyObject *sysPath = PySys_GetObject("path");

    file::path scripts = os_get_working_dir();
    scripts.combine_with("data/scripts");
    const char *scriptsPath = scripts.UnifiedPath.to_c_string(Context.TemporaryAlloc);
    PyList_Append(sysPath, PyUnicode_FromString(scriptsPath));

    auto filePath = scripts;
    filePath.combine_with(demo);
    if (!file::handle(filePath).is_file()) {
        fmt::print(">>>\n>>> Couldn't find file {!YELLOW}\"{}\"{!}.({!GRAY}\"{}\"{!})\n", filePath);
        return;
    }

    try {
        auto main = py::module::import(filePath.base_name().to_c_string(Context.TemporaryAlloc));
        GameState->PyModule = main;
        main.attr("load")((u64) GameState);
        GameState->PyFrame = (py::function) main.attr("frame");

        if (py::hasattr(main, "editor_variable")) {
            GameState->PyEditorVariable = (py::function) main.attr("editor_variable");

            if (GameState->EditorShowPositionalCorrection) {
                GameState->EditorPositionalCorrection = true;
                GameState->PyEditorVariable("positional_correction", true);
            }
            if (GameState->EditorShowIterations) {
                GameState->EditorIterations = 5;
                GameState->PyEditorVariable("iterations", 5);
            }
        }

        if (py::hasattr(main, "mouse_click")) GameState->PyMouseClick = (py::function) main.attr("mouse_click");
        if (py::hasattr(main, "mouse_release")) GameState->PyMouseRelease = (py::function) main.attr("mouse_release");
        if (py::hasattr(main, "mouse_move")) GameState->PyMouseMove = (py::function) main.attr("mouse_move");
    } catch (py::error_already_set e) {
        report_python_error(e);
    }

    GameState->PyLoaded = true;

    auto *cam = &GameState->Camera;
    camera_reinit(cam);
}

void refresh_python_demo_files() {
    file::path scripts = os_get_working_dir();
    scripts.combine_with("data/scripts");

    GameState->PyDemoFiles.reset();

    auto h = file::handle(scripts);
    if (!h.is_directory()) {
        GameState->PyLoaded = false;
        fmt::print(
            ">>>\n>>> Couldn't find {!YELLOW}\"data/scripts\"{!} folder in current working dir ({!GRAY}\"{}\"{!})\n",
            scripts);
        fmt::print(
            ">>> There must be a file named {!YELLOW}data/scripts/physics_main.py{!} relative to the current working "
            "directory in order to run.\n>>>\n");
        return;
    }

    h.traverse([](file::path file) {
        if (file.base_name().begins_with("demo_") && file.extension() == ".py")
            clone(GameState->PyDemoFiles.append(), file.file_name());
    });

    if (GameState->PyDemoFiles.Count == 0) {
        fmt::print(">>>\n>>> Couldn't find any demo files in {!YELLOW}\"data/scripts\"{!}. ({!GRAY}\"{}\"{!})\n",
                   scripts);
        fmt::print(">>> Demo files must be named like so: \"data/scripts/demo_*something*.py\"\n>>>\n");
    }
}

void report_python_error(py::error_already_set &e) {
    e.restore();

    fmt::print(">>> An {!RED}error{!} occured in python. Here is the stack trace:\n");
    PyErr_Print();
    fmt::print("\n");
}

void load_imgui_bindings_for_python() {
    GameState->ViewportAddLine = [](v2 p1, v2 p2, u32 color, f32 thickness) {
        GameState->ViewportDrawlist->AddLine(p1, p2, color, thickness);
    };
    GameState->ViewportAddRect = [](v2 p1, v2 p2, u32 color, f32 rounding, ImDrawCornerFlags_ cornerFlags,
                                    f32 thickness) {
        GameState->ViewportDrawlist->AddRect(p1, p2, color, rounding, cornerFlags, thickness);
    };
    GameState->ViewportAddRectFilled = [](v2 p1, v2 p2, u32 color, f32 rounding, ImDrawCornerFlags_ cornerFlags) {
        GameState->ViewportDrawlist->AddRectFilled(p1, p2, color, rounding, cornerFlags);
    };
    GameState->ViewportAddRectFilledMultiColor = [](v2 p1, v2 p2, u32 color_ul, u32 color_ur, u32 color_dr,
                                                    u32 color_dl) {
        GameState->ViewportDrawlist->AddRectFilledMultiColor(p1, p2, color_ul, color_ur, color_dr, color_dl);
    };
    GameState->ViewportAddQuad = [](v2 p1, v2 p2, v2 p3, v2 p4, u32 color, f32 thickness) {
        GameState->ViewportDrawlist->AddQuad(p1, p2, p3, p4, color, thickness);
    };
    GameState->ViewportAddQuadFilled = [](v2 p1, v2 p2, v2 p3, v2 p4, u32 color) {
        GameState->ViewportDrawlist->AddQuadFilled(p1, p2, p3, p4, color);
    };
    GameState->ViewportAddTriangle = [](v2 p1, v2 p2, v2 p3, u32 color, f32 thickness) {
        GameState->ViewportDrawlist->AddTriangle(p1, p2, p3, color, thickness);
    };
    GameState->ViewportAddTriangleFilled = [](v2 p1, v2 p2, v2 p3, u32 color) {
        GameState->ViewportDrawlist->AddTriangleFilled(p1, p2, p3, color);
    };
    GameState->ViewportAddCircle = [](v2 center, f32 radius, u32 color, s32 numSegments, f32 thickness) {
        GameState->ViewportDrawlist->AddCircle(center, radius, color, numSegments, thickness);
    };
    GameState->ViewportAddCircleFilled = [](v2 center, f32 radius, u32 color, s32 numSegments) {
        GameState->ViewportDrawlist->AddCircleFilled(center, radius, color, numSegments);
    };
    GameState->ViewportAddConvexPolyFilled = [](const f32 *data, s32 count, u32 color) {
        GameState->ViewportDrawlist->CUSTOM_AddConvexPolyFilled((const ImVec2 *) data, count, color,
                                                                GameState->PixelsPerMeter, -GameState->PixelsPerMeter);
    };
}
