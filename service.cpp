#include <os>

#include "Wargame/Content/Filesystem.hpp"
#include "Wargame/Game/Game.hpp"

namespace {
  auto &inet = net::Interfaces::get(0);

  using WebServer = std::unique_ptr<http::Server>;
  struct {
    WebServer httpServer, httpsServer;
  } webServers;

  void dispatchRequest(Networking::Request &request, Networking::Response &response) {
    auto route = Networking::router.route(request->method(), request->uri().to_string());
    route.handler(request, response);
  }
}

struct {} UseSSL;
struct {} NoSSL;

void Service::start() {
  inet.on_config([](net::Inet::Stack &) {
    auto makeServer = [](WebServer &server, auto ssl, int port) {
      if constexpr(std::is_same_v<decltype(ssl), decltype(UseSSL)>) {
        static_assert(std::is_same_v<decltype(Filesystem::initFs), std::monostate>,
          "Filesystem not loaded!");
        server = std::make_unique<http::OpenSSL_server>("/ssl.pem", "/ssl.key", inet.tcp());
      } else {
        server = std::make_unique<http::Server>(inet.tcp());
      }

      server->on_request([](Networking::Request request, Networking::Response response){
        dispatchRequest(request, response);
        response->write();
      });

      server->listen(port);
    };

    makeServer(webServers.httpServer, NoSSL, 80);
    makeServer(webServers.httpsServer, UseSSL, 443);
  });

	inet.negotiate_dhcp(3.0, [](bool timeout){
    if(timeout) {
      printf("DHCP negotiation failed, falling back to static assignment.\n");
      inet.network_config(
        {10,0,0,69},
        {255,255,255,0},
        {10,0,0,1},
        {1,1,1,1}
      );
    }
  });
}
