#include <le/all.hpp>
#include <le/entry_point.hpp>

using namespace le;

struct my_layer : layer {
    my_layer() {}
};

application *le::create_application() {
    auto *app = new application;
    app->Window = (new window)->initialize("Math grapher", 800, 600);

    // app->add_layer(New<Log_All_Events>());
    app->add_layer(new my_layer);

    return app;
}
