#pragma once

#include "application/application.hpp"

int main(int argc, char *argv[]) {
    // Other init code here...
    le::Application *app = le::create_application();
    app->run();
}