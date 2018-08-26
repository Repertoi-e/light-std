#pragma once

#include <game.h>

#include <SDL.h>
#include <SDL_syswm.h>

b32 imgui_process_sdl_event(SDL_Event *event);

void imgui_init_for_sdl(SDL_Window *window);
void imgui_new_sdl_frame(GameInput *input, SDL_Window *window);
