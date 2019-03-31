#pragma once

#include "application/application.hpp"

int main(int argc, byte *argv[]) {
    // Other init code here...
    le::application *app = le::create_application();
    app->run();
}