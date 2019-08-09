#pragma once

#include <lstd/basic.h>

#include "window.h"

// The permanent state of the game
struct game_memory {
    le::window *Window;
    allocator Allocator;
	
	// Any data that must be preserved through reloads.
    void *State = null;

    // Gets set to true when the game code has been reloaded during the frame.
	// Should be handled in _game_update_and_render_.
	// Gets triggered the first time the game loads as well!
    bool ReloadedThisFrame = false;
};

#define GAME_NEW(type) new (g_GameMemory->Allocator) type

#define GAME_UPDATE_AND_RENDER(name, ...) void name(game_memory *gameMemory)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_func);
