#include <le/all.hpp>
#include <le/entry_point.hpp>

#include <cmath>

using namespace le;

struct My_Layer : Layer {
    My_Layer() {}
};

Application *le::create_application() {
    auto *app = new Application;
    app->WindowPtr = (new Window)->initialize("Math grapher", 800, 600);

    // app->add_layer(New<Log_All_Events>());
    app->add_layer(new My_Layer);

    return app;
}
