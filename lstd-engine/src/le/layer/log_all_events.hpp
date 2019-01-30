#pragma once

#include "../application/Application.hpp"
#include "../event/event_fmt.hpp"

#include <lstd/io.hpp>

namespace le {
struct Log_All_Events : Layer {
    size_t e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14;

    void on_add() override {
        auto &window = *Application::get().WindowPtr;
        e1 = window.WindowClosedEvent.connect({this, &Log_All_Events::on_window_closed});
        e2 = window.WindowResizedEvent.connect({this, &Log_All_Events::on_window_resized});
        e3 = window.WindowGainedFocusEvent.connect({this, &Log_All_Events::on_window_gained_focus});
        e4 = window.WindowLostFocusEvent.connect({this, &Log_All_Events::on_window_lost_focus});
        e5 = window.WindowMovedEvent.connect({this, &Log_All_Events::on_window_moved});
        e6 = window.KeyPressedEvent.connect({this, &Log_All_Events::on_key_pressed});
        e7 = window.KeyReleasedEvent.connect({this, &Log_All_Events::on_key_released});
        e8 = window.KeyTypedEvent.connect({this, &Log_All_Events::on_key_typed});
        e9 = window.MouseButtonPressedEvent.connect({this, &Log_All_Events::on_mouse_button_pressed});
        e10 = window.MouseButtonReleasedEvent.connect({this, &Log_All_Events::on_mouse_button_released});
        e11 = window.MouseScrolledEvent.connect({this, &Log_All_Events::on_mouse_scrolled});
        e12 = window.MouseEnteredEvent.connect({this, &Log_All_Events::on_mouse_entered});
        e13 = window.MouseLeftEvent.connect({this, &Log_All_Events::on_mouse_left});
        e14 = window.MouseMovedEvent.connect({this, &Log_All_Events::on_mouse_moved});
    }
    void on_remove() override {
        auto &window = *Application::get().WindowPtr;
        window.WindowClosedEvent.disconnect(e1);
        window.WindowResizedEvent.disconnect(e2);
        window.WindowGainedFocusEvent.disconnect(e3);
        window.WindowLostFocusEvent.disconnect(e4);
        window.WindowMovedEvent.disconnect(e5);
        window.KeyPressedEvent.disconnect(e6);
        window.KeyReleasedEvent.disconnect(e7);
        window.KeyTypedEvent.disconnect(e8);
        window.MouseButtonPressedEvent.disconnect(e9);
        window.MouseButtonReleasedEvent.disconnect(e10);
        window.MouseScrolledEvent.disconnect(e11);
        window.MouseEnteredEvent.disconnect(e12);
        window.MouseLeftEvent.disconnect(e13);
        window.MouseMovedEvent.disconnect(e14);
    };

    void on_window_closed(const Window_Closed_Event &e) { fmt::print("{}\n", e); }
    void on_window_resized(const Window_Resized_Event &e) { fmt::print("{}\n", e); }
    void on_window_gained_focus(const Window_Gained_Focus_Event &e) { fmt::print("{}\n", e); }
    void on_window_lost_focus(const Window_Lost_Focus_Event &e) { fmt::print("{}\n", e); }
    void on_window_moved(const Window_Moved_Event &e) { fmt::print("{}\n", e); }

    bool on_key_pressed(const Key_Pressed_Event &e) {
        fmt::print("{}\n", e);
        return true;
    }
    void on_key_released(const Key_Released_Event &e) { fmt::print("{}\n", e); }
    bool on_key_typed(const Key_Typed_Event &e) {
        fmt::print("{}\n", e);
        return true;
    }

    bool on_mouse_button_pressed(const Mouse_Button_Pressed_Event &e) {
        fmt::print("{}\n", e);
        return true;
    }
    void on_mouse_button_released(const Mouse_Button_Released_Event &e) { fmt::print("{}\n", e); }
    bool on_mouse_scrolled(const Mouse_Scrolled_Event &e) {
        fmt::print("{}\n", e);
        return true;
    }
    void on_mouse_entered(const Mouse_Entered_Event &e) { fmt::print("{}\n", e); }
    void on_mouse_left(const Mouse_Left_Event &e) { fmt::print("{}\n", e); }
    bool on_mouse_moved(const Mouse_Moved_Event &e) {
        fmt::print("{}\n", e);
        return true;
    }
};
}  // namespace le