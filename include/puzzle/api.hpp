#pragma once

namespace httplib {
class Server;
}

namespace puzzle {

void register_routes(httplib::Server& server);

}  // namespace puzzle
