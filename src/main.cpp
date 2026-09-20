#include "puzzle/api.hpp"

#include <httplib.h>

#include <iostream>

int main() {
    httplib::Server server;
    puzzle::register_routes(server);

    std::cout << "Coco server listening on http://localhost:8080\n";
    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to start server on port 8080\n";
        return 1;
    }

    return 0;
}
