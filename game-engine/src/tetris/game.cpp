#if !defined LE_BUILDING_GAME
#error Error
#endif

#include <le/game.h>

#include <lstd/io/fmt.h>

using namespace le;

void on_window_closed(const window_closed_event &e) { fmt::print("{}\n", e); }
void on_window_resized(const window_resized_event &e) { fmt::print("{}\n", e); }
void on_window_gained_focus(const window_gained_focus_event &e) { fmt::print("{}\n", e); }
void on_window_lost_focus(const window_lost_focus_event &e) { fmt::print("{}\n", e); }
void on_window_moved(const window_moved_event &e) { fmt::print("{}\n", e); }

bool on_key_pressed(const key_pressed_event &e) {
    fmt::print("{}\n", e);
    return true;
}
void on_key_released(const key_released_event &e) { fmt::print("{}\n", e); }
bool on_key_typed(const key_typed_event &e) {
    fmt::print("{}\n", e);
    return true;
}

bool on_mouse_button_pressed(const mouse_button_pressed_event &e) {
    fmt::print("{}\n", e);
    return true;
}
void on_mouse_button_released(const mouse_button_released_event &e) { fmt::print("{}\n", e); }
bool on_mouse_scrolled(const mouse_scrolled_event &e) {
    fmt::print("{}\n", e);
    return true;
}
void on_mouse_entered(const mouse_entered_event &e) { fmt::print("{}\n", e); }
void on_mouse_left(const mouse_left_event &e) { fmt::print("{}\n", e); }
bool on_mouse_moved(const mouse_moved_event &e) {
    fmt::print("{}\n", e);
    return true;
}

struct game_state {
    u32 Counter = 0;

    // Event hooks
    size_t e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14;
};

game_memory *g_GameMemory = null;

extern "C" LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *gameMemory) {
    if (gameMemory->ReloadedThisFrame) {
        g_GameMemory = gameMemory;

        auto *state = (game_state *) gameMemory->State;

        if (!gameMemory->State) {
            // The first time we load, initialize the allocator and game state
            auto *allocatorData = new (Malloc) free_list_allocator_data;
            allocatorData->init(512_MiB, free_list_allocator_data::Find_First);
            gameMemory->Allocator = {free_list_allocator, allocatorData};
            gameMemory->State = state = GAME_NEW(game_state);
        } else {
            gameMemory->Window->WindowClosedEvent.disconnect(state->e1);
            gameMemory->Window->WindowResizedEvent.disconnect(state->e2);
            gameMemory->Window->WindowGainedFocusEvent.disconnect(state->e3);
            gameMemory->Window->WindowLostFocusEvent.disconnect(state->e4);
            gameMemory->Window->WindowMovedEvent.disconnect(state->e5);
            gameMemory->Window->KeyPressedEvent.disconnect(state->e6);
            gameMemory->Window->KeyReleasedEvent.disconnect(state->e7);
            gameMemory->Window->KeyTypedEvent.disconnect(state->e8);
            gameMemory->Window->MouseButtonPressedEvent.disconnect(state->e9);
            gameMemory->Window->MouseButtonReleasedEvent.disconnect(state->e10);
            gameMemory->Window->MouseScrolledEvent.disconnect(state->e11);
            gameMemory->Window->MouseEnteredEvent.disconnect(state->e12);
            gameMemory->Window->MouseLeftEvent.disconnect(state->e13);
            gameMemory->Window->MouseMovedEvent.disconnect(state->e14);
        }

        state->e1 = gameMemory->Window->WindowClosedEvent.connect(on_window_closed);
        state->e2 = gameMemory->Window->WindowResizedEvent.connect(on_window_resized);
        state->e3 = gameMemory->Window->WindowGainedFocusEvent.connect(on_window_gained_focus);
        state->e4 = gameMemory->Window->WindowLostFocusEvent.connect(on_window_lost_focus);
        state->e5 = gameMemory->Window->WindowMovedEvent.connect(on_window_moved);
        state->e6 = gameMemory->Window->KeyPressedEvent.connect(on_key_pressed);
        state->e7 = gameMemory->Window->KeyReleasedEvent.connect(on_key_released);
        state->e8 = gameMemory->Window->KeyTypedEvent.connect(on_key_typed);
        state->e9 = gameMemory->Window->MouseButtonPressedEvent.connect(on_mouse_button_pressed);
        state->e10 = gameMemory->Window->MouseButtonReleasedEvent.connect(on_mouse_button_released);
        state->e11 = gameMemory->Window->MouseScrolledEvent.connect(on_mouse_scrolled);
        state->e12 = gameMemory->Window->MouseEnteredEvent.connect(on_mouse_entered);
        state->e13 = gameMemory->Window->MouseLeftEvent.connect(on_mouse_left);
        state->e14 = gameMemory->Window->MouseMovedEvent.connect(on_mouse_moved);

        Context.init_temporary_allocator(1_MiB);
        fmt::print("Game code reloaded.\n");
    }

    auto *state = (game_state *) gameMemory->State;
    if (state->Counter % 60 == 0) {
        fmt::print("Counter!!!! {}\n", state->Counter);
    }
    ++state->Counter;

    Context.TemporaryAlloc.free_all();
}
