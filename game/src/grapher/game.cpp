#if !defined LE_BUILDING_GAME
#error Error
#endif

#include "state.h"

static void framebuffer_resized(const window_framebuffer_resized_event &e) {
    if (!GameMemory->MainWindow->is_visible()) return;

    State->ViewportRenderTarget.release();
    State->ViewportRenderTarget.init_as_render_target(Graphics, "Docked Viewport Render Target", e.Width, e.Height);
}

void release_state() {
    State->ViewportRenderTarget.release();
    if (State->FBSizeCBID != npos) {
        GameMemory->MainWindow->WindowFramebufferResizedEvent.disconnect(State->FBSizeCBID);
    }
    if (State->FocusCBID != npos) GameMemory->MainWindow->WindowFocusedEvent.disconnect(State->FocusCBID);
}

void reload_state() {
    release_state();

    assert(GameMemory->ImGuiContext);
    ImGui::SetCurrentContext((ImGuiContext *) GameMemory->ImGuiContext);
    ImGui::SetAllocatorFunctions([](size_t size, void *) { return operator new(size, Malloc); },
                                 [](void *ptr, void *) { delete ptr; });

    vec2i windowSize = GameMemory->MainWindow->get_size();
    framebuffer_resized({GameMemory->MainWindow, windowSize.x, windowSize.y});
    State->FBSizeCBID = GameMemory->MainWindow->WindowFramebufferResizedEvent.connect(framebuffer_resized);

    State->FocusCBID = GameMemory->MainWindow->WindowFocusedEvent.connect([](const auto &e) {
        if (!e.Focused && State->MouseGrabbed) {
            State->MouseGrabbed = false;
            GameMemory->MainWindow->set_cursor_mode(window::CURSOR_NORMAL);
        }
    });
}

void reload(game_memory *memory, graphics *g) {
    if (Context.Alloc.Function == Malloc.Function) {  // @Hack
        const_cast<implicit_context *>(&Context)->Alloc.Function = memory->ExeMalloc;
    }
    Malloc.Function = memory->ExeMalloc;

    GameMemory = memory;
    Graphics = g;

    if (!memory->Allocator) {
        auto *allocatorData = new (Malloc) free_list_allocator_data;
        allocatorData->init(128_MiB, free_list_allocator_data::Find_First);
        memory->Allocator = {free_list_allocator, allocatorData};
    }

    MANAGE_GLOBAL_STATE(State);
    MANAGE_GLOBAL_STATE(Shaders);

    reload_state();
}

LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *memory, graphics *g) {
    if (memory->ReloadedThisFrame) reload(memory, g);

    auto *win = GameMemory->MainWindow;
    if ((win->Keys[Key_LeftControl] || win->Keys[Key_RightControl]) && win->KeysThisFrame[Key_F]) {
        State->Editor = !State->Editor;
    }
    if (State->Editor) editor_main();

    update_and_render_scene();

    Context.TemporaryAlloc.free_all();
}
