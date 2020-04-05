#pragma once

#include <lstd/basic.h>
#include <lstd/catalog.h>
#include <lstd/graphics.h>
#include <lstd/io/fmt.h>
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
    allocator_func_t ExeMalloc = null;  // We need this because every time we hotload we construct a different malloc
                                        // and that doesn't work when freeing

    window *MainWindow = null;
    allocator Allocator;

    table<string, void *> States;

    void *ImGuiContext = null;

    // Gets set to true when the game code has been reloaded during the frame
    // (automatically set to false the next frame).
    // Gets triggered the first time the game loads as well!
    bool ReloadedThisFrame = false;
};

#define MANAGE_GLOBAL_STATE(state)                                      \
    if (!state) {                                                       \
        string identifier = #state;                                     \
        identifier.append("Ident");                                     \
        auto **found = GameMemory->States.find(identifier);             \
        if (!found) {                                                   \
            state = GAME_NEW(remove_pointer_t<decltype(state)>);        \
            GameMemory->States.move_add(&identifier, (void **) &state); \
        } else {                                                        \
            state = (decltype(state)) * found;                          \
        }                                                               \
    }

#define GAME_NEW(type) new (GameMemory->Allocator) type

#define GAME_UPDATE_AND_RENDER(name, ...) void name(game_memory *memory, graphics *g)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_func);

// Global, used in the game
inline game_memory *GameMemory = null;
inline graphics *Graphics = null;

inline catalog *AssetCatalog;
