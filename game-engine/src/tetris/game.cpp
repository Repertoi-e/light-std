#if !defined LE_BUILDING_GAME
#error Error
#endif

#include <le/game.h>

#include <lstd/io/fmt.h>

using namespace le;

struct game_state {
};

game_memory *g_GameMemory = null;

extern "C" LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *gameMemory) {
    if (gameMemory->ReloadedThisFrame) {
        g_GameMemory = gameMemory;

        auto *state = (game_state *) gameMemory->State;
        if (!state) {
            // The first time we load, initialize the allocator and game state
            auto *allocatorData = new (Malloc) free_list_allocator_data;
            allocatorData->init(512_MiB, free_list_allocator_data::Find_First);
            gameMemory->Allocator = {free_list_allocator, allocatorData};
            gameMemory->State = state = GAME_NEW(game_state);
        } else {

        }

        Context.init_temporary_allocator(1_MiB);
    }

    auto *state = (game_state *) gameMemory->State;

    Context.TemporaryAlloc.free_all();
}
