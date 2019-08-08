#pragma once

#include <lstd/basic.h>

#include "window.h"

// The permanent state of the game
struct game_memory {
    le::window *Window;
    allocator Allocator;

    // Used by the game for first-time run initialization
    void *State = null;
};

#define GAME_NEW(type) new (g_GameMemory->Allocator) type

#define GAME_UPDATE_AND_RENDER(name, ...) void name(game_memory *gameMemory)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_func);
