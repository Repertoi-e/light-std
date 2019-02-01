#pragma once

#include "../core.hpp"

#include <lstd/signal/signal.hpp>
#include <lstd/string/string.hpp>

#include "../events.hpp"

namespace le {

struct LE_API Window {
    string Title;
    s32 Left, Top;
    u32 Width, Height;
    bool VSyncEnabled = false;
    bool Closed = false;

    // Reserve 256 bytes for any platform data needed by implementations.
    byte PlatformData[256] = {};

    Window() {}

    // Returns this
    Window *initialize(const string &title, u32 width, u32 height);

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

    Signal<void(const Window_Closed_Event &)> WindowClosedEvent;
    Signal<void(const Window_Resized_Event &)> WindowResizedEvent;
    Signal<void(const Window_Gained_Focus_Event &)> WindowGainedFocusEvent;
    Signal<void(const Window_Lost_Focus_Event &)> WindowLostFocusEvent;
    Signal<void(const Window_Moved_Event &)> WindowMovedEvent;

    Signal<bool(const Key_Pressed_Event &), Collector_While0<bool>> KeyPressedEvent;
    Signal<void(const Key_Released_Event &)> KeyReleasedEvent;
    Signal<bool(const Key_Typed_Event &), Collector_While0<bool>> KeyTypedEvent;

    Signal<bool(const Mouse_Button_Pressed_Event &), Collector_While0<bool>> MouseButtonPressedEvent;
    Signal<void(const Mouse_Button_Released_Event &)> MouseButtonReleasedEvent;
    Signal<bool(const Mouse_Scrolled_Event &), Collector_While0<bool>> MouseScrolledEvent;
    Signal<void(const Mouse_Entered_Event &)> MouseEnteredEvent;
    Signal<void(const Mouse_Left_Event &)> MouseLeftEvent;
    Signal<bool(const Mouse_Moved_Event &), Collector_While0<bool>> MouseMovedEvent;

   private:
    void on_window_resized(const Window_Resized_Event &e) {
        Width = e.Width;
        Height = e.Height;
    }

    void on_window_moved(const Window_Moved_Event &e) {
        Left = e.Left;
        Top = e.Top;
    }
};
}  // namespace le