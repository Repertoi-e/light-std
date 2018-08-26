#include "imgui_setup_sdl.h"

#include <imgui/imgui.h>

#include "types.h"

static SDL_Cursor *g_MouseCursors[ImGuiMouseCursor_COUNT] = { 0 };

b32 imgui_process_sdl_event(SDL_Event *event) {
    ImGuiIO &io = ImGui::GetIO();
    if (event->type == SDL_MOUSEWHEEL) {
        if (event->wheel.x > 0) io.MouseWheelH += 1;
        if (event->wheel.x < 0) io.MouseWheelH -= 1;
        if (event->wheel.y > 0) io.MouseWheel  += 1;
        if (event->wheel.y < 0) io.MouseWheel  -= 1;
        return true;
    } else if (event->type == SDL_TEXTINPUT) {
        io.AddInputCharactersUTF8(event->text.text);
        return true;
    } else if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        SDL_Keysym keysym = event->key.keysym;

        SDL_assert(keysym.scancode >= 0 && keysym.scancode < ArrayCount(io.KeysDown));
        io.KeysDown[keysym.scancode] = (event->type == SDL_KEYDOWN);

        io.KeyShift = keysym.mod & KMOD_SHIFT;
        io.KeyCtrl  = keysym.mod & KMOD_CTRL;
        io.KeyAlt   = keysym.mod & KMOD_ALT;
        io.KeySuper = keysym.mod & KMOD_GUI;
        return true;
    }
    return false;
}

void imgui_init_for_sdl(SDL_Window *window) {
    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab]        = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp]     = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown]   = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home]       = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End]        = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Insert]     = SDL_SCANCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete]     = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Space]      = SDL_SCANCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter]      = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape]     = SDL_SCANCODE_ESCAPE;

    io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

    io.SetClipboardTextFn = [](void *userData, char const *text) {
        SDL_SetClipboardText(text);
    };
    io.GetClipboardTextFn = [](void *userData) -> char const* {
        return SDL_GetClipboardText();
    };
    io.ClipboardUserData = 0;

    g_MouseCursors[ImGuiMouseCursor_Arrow]      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    g_MouseCursors[ImGuiMouseCursor_TextInput]  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    g_MouseCursors[ImGuiMouseCursor_ResizeAll]  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    g_MouseCursors[ImGuiMouseCursor_ResizeNS]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    g_MouseCursors[ImGuiMouseCursor_ResizeEW]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
    g_MouseCursors[ImGuiMouseCursor_Hand]       = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    // Setup IME for windows
#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);

    io.ImeWindowHandle = wmInfo.info.win.window;
#endif
}

static void imgui_update_mouse_pos_and_buttons_for_sdl(GameInput *input, SDL_Window *window) {
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantSetMousePos) {
        SDL_WarpMouseInWindow(window, (int) io.MousePos.x, (int) io.MousePos.y);
    } else {
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }

    // If a mouse press event came, always pass it as "mouse held this frame", 
    // so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[0] = input->MouseButtons[SDL_BUTTON_LEFT].EndedDown;
    io.MouseDown[1] = input->MouseButtons[SDL_BUTTON_RIGHT].EndedDown;
    io.MouseDown[2] = input->MouseButtons[SDL_BUTTON_MIDDLE].EndedDown;

#if !defined __EMSCRIPTEN__
    if (window == SDL_GetKeyboardFocus()) {
        // SDL_GetMouseState() gives mouse position seemingly based on the last window entered/focused(?)
        // The creation of a new windows at runtime and SDL_CaptureMouse both seems to severely mess up with that,
        // so we retrieve that position globally.
        int mouseX, mouseY, mouseWindowX, mouseWindowY;
        SDL_GetWindowPosition(window, &mouseWindowX, &mouseWindowY);
        SDL_GetGlobalMouseState(&mouseX, &mouseY);
        mouseX -= mouseWindowX;
        mouseY -= mouseWindowY;
        io.MousePos = ImVec2((f32) mouseX, (f32) mouseY);
    }

    // SDL_CaptureMouse() lets the OS know e.g. that our imgui drag outside the SDL window
    // boundaries shouldn't e.g. trigger the OS window resize cursor. 
    SDL_CaptureMouse((b32) ImGui::IsAnyMouseDown());
#else
    if (SDL_GetWindowFlags(g_Window) & SDL_WINDOW_INPUT_FOCUS) {
        io.MousePos = ImVec2((f32) input->MouseX, (f32) input->MouseY);
    }
#endif
}

// Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
static u64 g_Frequency = SDL_GetPerformanceFrequency();
static u64 g_LastTime = 0;

void imgui_new_sdl_frame(GameInput *input, SDL_Window *window) {
    ImGuiIO &io = ImGui::GetIO();

    SDL_assert(io.Fonts->IsBuilt());

    // Setup display size (every frame to accommodate for window resizing)
    s32 width, height;
    SDL_GetWindowSize(window, &width, &height);

    s32 displayWidth, displayHeight;
    SDL_GL_GetDrawableSize(window, &displayWidth, &displayHeight);

    io.DisplaySize = ImVec2((f32) width, (f32) height);
    io.DisplayFramebufferScale = ImVec2(width > 0 ? ((f32) displayWidth / width) : 0, height > 0 ? ((f32) displayHeight / height) : 0);

    u64 currentTime = SDL_GetPerformanceCounter();
    io.DeltaTime = g_LastTime > 0 ? (f32) ((f64) (currentTime - g_LastTime) / g_Frequency) : (1.0f / 60.0f);
    g_LastTime = currentTime;

    imgui_update_mouse_pos_and_buttons_for_sdl(input, window);

    // Update the mouse cursor
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0) {
        ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
        if (io.MouseDrawCursor || imguiCursor == ImGuiMouseCursor_None) {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            SDL_ShowCursor(SDL_FALSE);
        } else {
            SDL_Cursor *cursor = g_MouseCursors[ImGuiMouseCursor_Arrow];
            if (g_MouseCursors[imguiCursor]) {
                cursor = g_MouseCursors[imguiCursor];
            }
            SDL_SetCursor(cursor);
            SDL_ShowCursor(SDL_TRUE);
        }
    }
}