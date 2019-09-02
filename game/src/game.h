#pragma once

#include <lstd/basic.h>
#include <lstd/graphics/graphics.h>
#include <lstd/os.h>

// LE_GAME_API is used to export functions from the game dll
#if OS == WINDOWS
#if defined LE_BUILDING_GAME
#define LE_GAME_API extern "C" __declspec(dllexport)
#else
#define LE_GAME_API __declspec(dllimport)
#endif
#else
#if COMPILER == GCC || COMPILER == CLANG
#if defined LE_BUILDING_GAME
#define LE_GAME_API __attribute__((visibility("default")))
#else
#define LE_GAME_API
#endif
#else
#define LE_GAME_API
#pragma warning Unknown dynamic link import / export semantics.
#endif
#endif

// 'x' needs to have dll-interface to be used by clients of struct 'y'
// This will never be a problem since nowhere do we change struct sizes based on debug/release/whatever conditions
#if COMPILER == MSVC
#pragma warning(disable : 4251)
#endif

// The permanent state of the game
struct game_memory {
    window *MainWindow = null;
    allocator Allocator;

    void *ImGuiContext = null;

    // Any data that must be preserved through reloads
    void *State = null;

    // Gets set to true when the game code has been reloaded during the frame.
    // Should be handled in _game_update_and_render_.
    // Gets triggered the first time the game loads as well!
    bool ReloadedThisFrame = false;
};

#define GAME_NEW(type) new (g_GameMemory->Allocator) type

#define GAME_UPDATE_AND_RENDER(name, ...) void name(game_memory *memory, graphics *g)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_func);

#define GAME_RENDER_UI(name, ...) void name()
typedef GAME_RENDER_UI(game_render_ui_func);
