#if !defined LE_BUILDING_GAME
#error Error
#endif

#include "state.h"

static void framebuffer_resized(const window_framebuffer_resized_event &e) {
    if (!GameMemory->MainWindow->is_visible()) return;

    GameState->ViewportRenderTarget.release();
    GameState->ViewportRenderTarget.init_as_render_target(Graphics, e.Width, e.Height);
}

void release_state() {
    GameState->ViewportRenderTarget.release();
    if (GameState->FBSizeCBID != npos) {
        GameMemory->MainWindow->WindowFramebufferResizedEvent.disconnect(GameState->FBSizeCBID);
    }
    if (GameState->FocusCBID != npos) GameMemory->MainWindow->WindowFocusedEvent.disconnect(GameState->FocusCBID);
}

void reload_game_state() {
    release_state();

    assert(GameMemory->ImGuiContext);
    ImGui::SetCurrentContext((ImGuiContext *) GameMemory->ImGuiContext);
    ImGui::SetAllocatorFunctions([](size_t size, void *) { return operator new(size, Malloc); },
                                 [](void *ptr, void *) { delete ptr; });

    vec2<s32> windowSize = GameMemory->MainWindow->get_size();
    framebuffer_resized({GameMemory->MainWindow, windowSize.x, windowSize.y});
    GameState->FBSizeCBID = GameMemory->MainWindow->WindowFramebufferResizedEvent.connect(framebuffer_resized);

    GameState->FocusCBID = GameMemory->MainWindow->WindowFocusedEvent.connect([](const auto &e) {
        if (!e.Focused && GameState->MouseGrabbed) {
            GameState->MouseGrabbed = false;
            GameMemory->MainWindow->set_cursor_mode(window::CURSOR_NORMAL);
        }
    });
}

LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *memory, graphics *g) {
    if (memory->ReloadedThisFrame) {
        if (Context.Alloc.Function == Malloc.Function) {  // @Hack
            const_cast<implicit_context *>(&Context)->Alloc.Function = memory->ExeMalloc;
        }
        Malloc.Function = memory->ExeMalloc;

        GameMemory = memory;
        Graphics = g;

        if (!memory->Alloc) {
            auto *allocatorData = new (Malloc) free_list_allocator_data;
            allocatorData->init(128_MiB, free_list_allocator_data::Find_First);
            memory->Alloc = {free_list_allocator, allocatorData};
        }
        reload_global_state();
        reload_game_state();
    }

    static camera::type oldCameraType = Scene->Camera.Type;

    auto *win = GameMemory->MainWindow;
    if ((win->Keys[Key_LeftControl] || win->Keys[Key_RightControl]) && win->KeysThisFrame[Key_F]) {
        GameState->Editor = !GameState->Editor;
        
        // Ensure we use the FPS camera when we are not in the editor
        if (GameState->Editor) {
            Scene->Camera.Type = oldCameraType;
        } else {
            oldCameraType = Scene->Camera.Type;
            Scene->Camera.Type = camera::FPS;
        }
        
        if (GameState->MouseGrabbed) {
            GameState->MouseGrabbed = false;
            win->set_cursor_mode(window::CURSOR_NORMAL);
        }
    }

    // The viewport window may not be in an additional imgui window since we don't allow moving it,
    // so assuming it's in the main window's viewport is fine.
    if (GameState->MouseGrabbed && ImGui::IsKeyPressed(Key_Escape, false)) {
        GameState->MouseGrabbed = false;
        win->set_cursor_mode(window::CURSOR_NORMAL);
    }

    if (!GameState->Editor) {
        if (win->is_hovered() && win->MouseButtons[Mouse_Button_Left]) {
            GameState->MouseGrabbed = true;
            win->set_cursor_mode(window::CURSOR_DISABLED);
        }
    }

    if (GameState->Editor) editor_main();

    update_and_render_scene();

    Context.TemporaryAlloc.free_all();
}
