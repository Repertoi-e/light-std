#if !defined LE_BUILDING_GAME
#error Error
#endif

#include <le/game.h>

#include <lstd/io/fmt.h>

using namespace le;

game_memory *g_GameMemory = null;

struct game_state {
    u32 Counter = 0;
};

extern "C" LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *gameMemory) {
    if (!g_GameMemory) {
        g_GameMemory = gameMemory;
        Context.init_temporary_allocator(1_MiB);

        if (!gameMemory->State) {
            auto *allocatorData = new (Malloc) free_list_allocator_data;
            allocatorData->init(512_MiB, free_list_allocator_data::Find_First);
            gameMemory->Allocator = {free_list_allocator, allocatorData};

            gameMemory->State = GAME_NEW(game_state);
        }
    }

    auto *state = (game_state *) gameMemory->State;
    if (state->Counter % 60 == 0) {
        fmt::print("Counter: {}\n", state->Counter);
    }
    ++state->Counter;

    Context.TemporaryAlloc.free_all();
}
