#include <le/all.hpp>
#include <le/entry_point.hpp>

using namespace le;

struct My_Layer : Layer {
    string Title;

    void on_add() override {
        Title = Application::get().WindowPtr->Title;
    }

    void on_update(f32 dt) override {
        auto &window = *Application::get().WindowPtr;
        window.set_title(Title + " | Windows");
    }
};

Application *le::create_application() {
    auto *app = New<Application>();
    app->WindowPtr = New<Window>()->initialize("Math grapher", 1280, 720);

    // app->add_layer(New<Log_All_Events>());
    app->add_layer(New<My_Layer>());

    return app;
}
