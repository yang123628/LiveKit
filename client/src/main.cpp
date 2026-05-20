#include "app/Application.h"

int main(int argc, char* argv[]) {
    auto& app = Application::instance();
    app.initialize(argc, argv);
    return app.run();
}
