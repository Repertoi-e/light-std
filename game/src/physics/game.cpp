#if !defined LE_BUILDING_GAME
#error Error
#endif

#include "state.h"

void reload_game_state() {
    assert(GameMemory->ImGuiContext);
    ImGui::SetCurrentContext((ImGuiContext *) GameMemory->ImGuiContext);
    ImGui::SetAllocatorFunctions([](size_t size, void *) { return operator new(size, Malloc); },
                                 [](void *ptr, void *) { delete ptr; });
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

    auto *win = GameMemory->MainWindow;
    if ((win->Keys[Key_LeftControl] || win->Keys[Key_RightControl]) && win->KeysThisFrame[Key_F]) {
        GameState->Editor = !GameState->Editor;
    }

    if (GameMemory->MainWindow->is_visible() && GameState->Editor) {
        editor_main();
    }
    update_and_render_scene();

    Context.TemporaryAlloc.free_all();
}
