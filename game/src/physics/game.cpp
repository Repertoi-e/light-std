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
        Malloc = memory->Alloc;  // Should we? Might be a bit confusing..

        // We need to use the exe's imgui context, because we submit the geometry to the GPU there
        assert(GameMemory->ImGuiContext);
        ImGui::SetCurrentContext((ImGuiContext *) GameMemory->ImGuiContext);

        // We also tell imgui to use our allocator (by default it uses raw malloc)
        ImGui::SetAllocatorFunctions([](size_t size, void *) { return operator new(size); },
                                     [](void *ptr, void *) { delete ptr; });

        reload_global_state();
    }

    GameState->Camera.update();

    if (GameMemory->MainWindow->is_visible()) {
        editor_main();
        editor_scene_properties();
    }

    viewport_render();

    Context.TemporaryAlloc.free_all();
}
