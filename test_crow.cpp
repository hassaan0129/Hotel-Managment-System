#define CROW_MAIN
#include "crow_all.h"
#include <iostream>

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello from Crow!";
    });

    std::cout << "🚀 Starting Crow server on port 18080..." << std::endl;

    app.port(18080).multithreaded().run();
}
