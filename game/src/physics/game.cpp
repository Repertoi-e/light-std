#if !defined LE_BUILDING_GAME
#error Error
#endif

#include "state.h"

LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *memory, graphics *g) {
    if (memory->ReloadedThisFrame) {
        GameMemory = memory;
        Graphics = g;

        // Switch our default allocator from malloc to the one the exe provides us with
        Context.Alloc = memory->Alloc;

        // We need to use the exe's imgui context, because we submit the geometry to the GPU there
        assert(GameMemory->ImGuiContext);
        ImGui::SetCurrentContext((ImGuiContext *) GameMemory->ImGuiContext);

        // We also tell imgui to use our allocator (we tell imgui to not implement default ones)
        ImGui::SetAllocatorFunctions([](size_t size, void *) { return operator new(size, GameMemory->Alloc); },
                                     [](void *ptr, void *) { delete ptr; });

        reload_global_state();

        auto *cam = &GameState->Camera;
        camera_reinit(cam);
    }

    auto *cam = &GameState->Camera;
    camera_update(cam);

    if (GameMemory->MainWindow->is_visible()) {
        editor_main();
        editor_scene_properties();
        if (memory->ReloadedThisFrame) ImGui::SetWindowFocus("Python");
    }

    viewport_render();

    Context.TemporaryAlloc.free_all();
}

LE_GAME_API GAME_MAIN_WINDOW_EVENT(game_main_window_event, const event &e) {
    if (!GameState) return false;

    if (e.Type == event::Mouse_Button_Pressed && !GameMemory->MainWindow->Keys[Key_LeftControl]) {
        // Our viewport coordinates are in monitor space
        v2 mouse = (v2)(e.Window->get_cursor_pos() + e.Window->get_pos());

        v2 p1 = GameState->ViewportPos;
        v2 p2 = GameState->ViewportPos + GameState->ViewportSize;
        if (mouse.x > p1.x && mouse.y > p1.y && mouse.x < p2.x && mouse.y < p2.y) {
            auto mouseInWorld = dot(mouse, GameState->InverseViewMatrix) / GameState->PixelsPerMeter;
            try {
                if (e.Button == Mouse_Button_Left) {
                    if (GameState->PyMouseClick) GameState->PyMouseClick(mouseInWorld.x, -mouseInWorld.y, false);
                } else if (e.Button == Mouse_Button_Right) {
                    if (GameState->PyMouseClick) GameState->PyMouseClick(mouseInWorld.x, -mouseInWorld.y, true);
                }
            } catch (py::error_already_set &e) {
                report_python_error(e);
            }
        }
    } else if (e.Type == event::Mouse_Button_Released) {
        try {
            if (e.Button == Mouse_Button_Left) {
                if (GameState->PyMouseRelease) GameState->PyMouseRelease(false);
            } else if (e.Button == Mouse_Button_Right) {
                if (GameState->PyMouseRelease) GameState->PyMouseRelease(true);
            }
        } catch (py::error_already_set &e) {
            report_python_error(e);
        }
    } else if (e.Type == event::Mouse_Moved) {
        // Our viewport coordinates are in monitor space
        v2 mouse = (v2)(e.Window->get_cursor_pos() + e.Window->get_pos());

        auto mouseInWorld = dot(mouse, GameState->InverseViewMatrix) / GameState->PixelsPerMeter;
        try {
            if (GameState->PyMouseMove) GameState->PyMouseMove(mouseInWorld.x, -mouseInWorld.y);
        } catch (py::error_already_set &e) {
            report_python_error(e);
        }
    }
    return false;
}
