#pragma once

#include "../core.hpp"

#include <lstd/signal/signal.hpp>
#include <lstd/string/string.hpp>

#include "../events.hpp"

namespace le {

struct LE_API window {
    string Title;
    s32 Left = 0, Top = 0;
    u32 Width = 0, Height = 0;
    bool VSyncEnabled = false;
    bool Closed = false;

    // Reserve 256 bytes for any platform data needed by implementations.
    byte PlatformData[256] = {};

    window() = default;

    // Returns this
    window *initialize(const string &title, u32 width, u32 height);

    void update();

    void set_title(const string &title);
    void set_vsync(bool enabled);
    void set_left(s32 left);
    void set_top(s32 top);
    void set_width(u32 width);
    void set_height(u32 height);

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