#pragma once

#include "types.h"

#include "memory/memory.h"
#include "memory/freelist_allocator.h"
#include "memory/linear_allocator.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_audio.h>

#include <ctime>

struct GameMemory {
    SDL_Window *SDLWindow;

    // General purpose allocator
    FreeListAllocator Permanent;

    // No free, only alloc, gets cleared at the end of a frame
    LinearAllocator Temporary;
};

struct GameButtonState {
    s32 HalfTransitionCount;
    s32 EndedDown;
};

struct GameInput {
    f32 DeltaTime;
    s32 MouseX, MouseY;
    // In order: NULL, LB, MB, RB, X1, X2. Use SDL_BUTTON_LEFT, etc.. to access to avoid confusion
    GameButtonState MouseButtons[6];

    union {
        GameButtonState Buttons[6];
        struct {
            GameButtonState MoveForward;
            GameButtonState MoveBackward;
            GameButtonState MoveLeft;
            GameButtonState MoveRight;
            GameButtonState Sprint;
            GameButtonState Jump;
        };
    };
};

#define GAME_PROCESS_SDL_EVENT(name, ...) void name(GameMemory *gameMemory, GameInput *input, SDL_Event *event)
typedef GAME_PROCESS_SDL_EVENT(game_process_sdl_event_func);

#define GAME_UPDATE_AND_RENDER(name, ...) void name(GameMemory *gameMemory, GameInput *input)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_func);

struct GameCode {
    void *Object;
    
    game_process_sdl_event_func *ProcessSDLEvent;
    game_update_and_render_func *UpdateAndRender;

    std::time_t LastWriteTime;
    b32 IsValid;
};