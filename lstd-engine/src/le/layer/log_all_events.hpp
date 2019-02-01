#pragma once

#include "../application/Application.hpp"
#include "../events_fmt.hpp"

#include <lstd/io.hpp>

namespace le {

void log_all_events_layer_on_add(void *data);
void log_all_events_layer_on_remove(void *data);

struct Log_All_Events : Layer {
    size_t e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14;

    Log_All_Events() {
        on_add_function = log_all_events_layer_on_add;
        on_remove_function = log_all_events_layer_on_remove;
    }

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

void log_all_events_layer_on_add(void *data) {
    auto *layer = (Log_All_Events *) data;

    auto &window = *Application::get().WindowPtr;
    layer->e1 = window.WindowClosedEvent.connect({layer, &Log_All_Events::on_window_closed});
    layer->e2 = window.WindowResizedEvent.connect({layer, &Log_All_Events::on_window_resized});
    layer->e3 = window.WindowGainedFocusEvent.connect({layer, &Log_All_Events::on_window_gained_focus});
    layer->e4 = window.WindowLostFocusEvent.connect({layer, &Log_All_Events::on_window_lost_focus});
    layer->e5 = window.WindowMovedEvent.connect({layer, &Log_All_Events::on_window_moved});
    layer->e6 = window.KeyPressedEvent.connect({layer, &Log_All_Events::on_key_pressed});
    layer->e7 = window.KeyReleasedEvent.connect({layer, &Log_All_Events::on_key_released});
    layer->e8 = window.KeyTypedEvent.connect({layer, &Log_All_Events::on_key_typed});
    layer->e9 = window.MouseButtonPressedEvent.connect({layer, &Log_All_Events::on_mouse_button_pressed});
    layer->e10 = window.MouseButtonReleasedEvent.connect({layer, &Log_All_Events::on_mouse_button_released});
    layer->e11 = window.MouseScrolledEvent.connect({layer, &Log_All_Events::on_mouse_scrolled});
    layer->e12 = window.MouseEnteredEvent.connect({layer, &Log_All_Events::on_mouse_entered});
    layer->e13 = window.MouseLeftEvent.connect({layer, &Log_All_Events::on_mouse_left});
    layer->e14 = window.MouseMovedEvent.connect({layer, &Log_All_Events::on_mouse_moved});
}

void log_all_events_layer_on_remove(void *data) {
    auto *layer = (Log_All_Events *) data;

    auto &window = *Application::get().WindowPtr;
    window.WindowClosedEvent.disconnect(layer->e1);
    window.WindowResizedEvent.disconnect(layer->e2);
    window.WindowGainedFocusEvent.disconnect(layer->e3);
    window.WindowLostFocusEvent.disconnect(layer->e4);
    window.WindowMovedEvent.disconnect(layer->e5);
    window.KeyPressedEvent.disconnect(layer->e6);
    window.KeyReleasedEvent.disconnect(layer->e7);
    window.KeyTypedEvent.disconnect(layer->e8);
    window.MouseButtonPressedEvent.disconnect(layer->e9);
    window.MouseButtonReleasedEvent.disconnect(layer->e10);
    window.MouseScrolledEvent.disconnect(layer->e11);
    window.MouseEnteredEvent.disconnect(layer->e12);
    window.MouseLeftEvent.disconnect(layer->e13);
    window.MouseMovedEvent.disconnect(layer->e14);
};
}  // namespace le