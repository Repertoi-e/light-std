#include <GL/glew.h>

#include "game.h"

#include "imgui_setup_sdl.h"
#include "imgui_setup_opengl.h"

struct GameState {
    ImGuiOpenGLState ImGuiGLState;
    ImVec4 ClearColor = ImVec4(0.2f, 0.3f, 0.8f, 1.0f);

    b32 IsInitted = false;
};

extern "C" DECLSPEC GAME_PROCESS_SDL_EVENT(game_process_sdl_event, GameMemory *gameMemory, GameInput *input, SDL_Event *event) {
    if (!imgui_process_sdl_event(event)) {
        // ... //
    }
}

static ImVector<char> *g_ImGuiLocalGlobalBuffer; // ... I.. Don't even...

extern "C" DECLSPEC GAME_UPDATE_AND_RENDER(game_update_and_render, GameMemory *gameMemory, GameInput *input) {
    GameState *state = (GameState *) ((u8 *) gameMemory->Permanent.Memory + sizeof(size_t) + 1);
    // TODO:                                                    PLEASE!   ^^^^^^^^^^^^^^^^^^^^
    if (!state->IsInitted) {
        SDL_assert(state == gameMemory->Permanent.alloc(sizeof(GameState)));

        g_ImGuiLocalGlobalBuffer = (ImVector<char> *) gameMemory->Permanent.alloc(sizeof(ImVector<char>));
        g_ImGuiLocalGlobalBuffer->Size = g_ImGuiLocalGlobalBuffer->Capacity = 0;
        g_ImGuiLocalGlobalBuffer->Data = 0;

        if (glewInit() != GLEW_OK) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "GLEW Init failed!\n");
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        imgui_init_for_sdl(gameMemory->SDLWindow);
        imgui_create_opengl_device_objects(&state->ImGuiGLState);

        ImGui::StyleColorsDark();

        SDL_Log("******************* OpenGL *******************\n");
        SDL_Log("* Vendor:     %s\n", glGetString(GL_VENDOR));
        SDL_Log("* Renderer:   %s\n", glGetString(GL_RENDERER));
        SDL_Log("* Version:    %s\n", glGetString(GL_VERSION));
        SDL_Log("**********************************************\n");

        state->IsInitted = true;
    }

    int width, height;
    SDL_GetWindowSize(gameMemory->SDLWindow, &width, &height);
    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(state->ClearColor.x, state->ClearColor.y, state->ClearColor.z, 1.0f);

    imgui_new_sdl_frame(input, gameMemory->SDLWindow);
    ImGui::NewFrame();

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::ColorEdit3("Clear color", (f32 *) &state->ClearColor);
    ImGui::Render();

    imgui_render_data_with_opengl(&state->ImGuiGLState, ImGui::GetDrawData());
}