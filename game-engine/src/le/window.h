#pragma once

#include <lstd/basic.h>

#include "core.h"
#include "events.h"

namespace le {

struct window {
    string Title;
    s32 Left = 0, Top = 0;
    u32 Width = 0, Height = 0;
    bool VSyncEnabled = false;
    bool Closed = false;

    // Reserve 256 bytes for any platform data needed by implementations.
    char PlatformData[256]{};

    window() = default;

    // Returns this
    window *init(string title, u32 width, u32 height, bool vsync);

    void update();

    // Call this to update the title of the window after _Title_ member has changed
    void update_title();

    // Call this to update the bounds of the window after _Left_ _Top _Width_ or _Height_ has changed
    void update_bounds();

    // Toggle vsync @TODO
    // void set_vsync(bool enabled);

    // Event signals. Connect to these to receive callbacks for this window.
    // Callbacks with return type of bool means whether the event has been handled.
    // This is useful for example when you want to stop the mouse left click event
    // passing "through" the UI onto the game world.
    // Returning true means stop emitting the event to the other callbacks.

    signal<void(const window_closed_event &)> WindowClosedEvent;
    signal<void(const window_resized_event &)> WindowResizedEvent;
    signal<void(const window_gained_focus_event &)> WindowGainedFocusEvent;
    signal<void(const window_lost_focus_event &)> WindowLostFocusEvent;
    signal<void(const window_moved_event &)> WindowMovedEvent;

    signal<bool(const key_pressed_event &), collector_while0<bool>> KeyPressedEvent;
    signal<void(const key_released_event &)> KeyReleasedEvent;
    signal<bool(const key_typed_event &), collector_while0<bool>> KeyTypedEvent;

    signal<bool(const mouse_button_pressed_event &), collector_while0<bool>> MouseButtonPressedEvent;
    signal<void(const mouse_button_released_event &)> MouseButtonReleasedEvent;
    signal<bool(const mouse_scrolled_event &), collector_while0<bool>> MouseScrolledEvent;
    signal<void(const mouse_entered_event &)> MouseEnteredEvent;
    signal<void(const mouse_left_event &)> MouseLeftEvent;
    signal<bool(const mouse_moved_event &), collector_while0<bool>> MouseMovedEvent;

   private:
    void on_window_resized(const window_resized_event &e) {
        Width = e.Width;
        Height = e.Height;
    }

    void on_window_moved(const window_moved_event &e) {
        Left = e.Left;
        Top = e.Top;
    }
};
}  // namespace le