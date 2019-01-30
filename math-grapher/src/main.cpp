#include <le/entry_point.hpp>
#include <le/event/event_fmt.hpp>

#include <lstd/io.hpp>

using namespace le;

struct My_Layer : Layer {
    s32 a = 2;

    void on_add() override {
        auto &window = *Application::get().WindowPtr;
        window.WindowClosedEvent.connect({this, &My_Layer::on_window_closed});
        window.WindowResizedEvent.connect({this, &My_Layer::on_window_resized});
        window.WindowGainedFocusEvent.connect({this, &My_Layer::on_window_gained_focus});
        window.WindowLostFocusEvent.connect({this, &My_Layer::on_window_lost_focus});
        window.WindowMovedEvent.connect({this, &My_Layer::on_window_moved});
        window.KeyPressedEvent.connect({this, &My_Layer::on_key_pressed});
        window.KeyReleasedEvent.connect({this, &My_Layer::on_key_released});
        window.KeyTypedEvent.connect({this, &My_Layer::on_key_typed});
        window.MouseButtonPressedEvent.connect({this, &My_Layer::on_mouse_button_pressed});
        window.MouseButtonReleasedEvent.connect({this, &My_Layer::on_mouse_button_released});
        window.MouseScrolledEvent.connect({this, &My_Layer::on_mouse_scrolled});
        window.MouseEnteredEvent.connect({this, &My_Layer::on_mouse_entered});
        window.MouseLeftEvent.connect({this, &My_Layer::on_mouse_left});
        window.MouseMovedEvent.connect({this, &My_Layer::on_mouse_moved});
    }
    void on_remove() override{};

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

Application *le::create_application() {
    auto *app = New<Application>();
    app->WindowPtr = New<Window>()->initialize("Math grapher", 1280, 720);

    app->add_layer(New<My_Layer>());
    return app;
}
