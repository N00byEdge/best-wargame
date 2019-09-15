#include <os>
#include <https>
#include <net/interfaces>

#include <mana/middleware/cookie_parser.hpp>

#include "Wargame/Content/Filesystem.hpp"
#include "Wargame/Game/Game.hpp"
#include "Wargame/Web/Middleware.hpp"

namespace {
  auto &inet = net::Interfaces::get(0);

  using WebServer = std::unique_ptr<http::Server>;
  struct {
    WebServer httpServer, httpsServer;
  } webServers;
  
  void dispatchRequest(Networking::Request &&request, Networking::Response &&response) {
    Wargame::middleware(request, response,
    [&](){
      try {
        auto route = Networking::router.match(request->method(), request->uri().to_string());
        route.job(request, response);
      }
      catch(mana::Router_error) {
        auto &stream = request->get_attribute<Wargame::StreamAttribute>()->ss;
        HTML::errorPage(stream, "Page not found.");
      }
    });
  }
}

struct {} UseSSL;
struct {} NoSSL;

void Service::start() {
  Networking::router.optimize_route_search();

  inet.on_config(
  [](net::Inet::Stack &) {
    auto makeServer = [](WebServer &server, auto ssl, int port) {
      if constexpr(std::is_same_v<decltype(ssl), decltype(UseSSL)>) {
        static_assert(std::is_same_v<decltype(Filesystem::initFs), std::monostate>,
          "Filesystem not loaded!");
        server = std::make_unique<http::OpenSSL_server>("/ssl.pem", "/ssl.key", inet.tcp());
      } else {
        server = std::make_unique<http::Server>(inet.tcp());
      }

      server->on_request(
      [](http::Request_ptr request, http::Response_writer_ptr response) {
        dispatchRequest(
          std::make_shared<mana::Request>(std::move(request)),
          std::make_shared<mana::Response>(std::move(response))
        );
      });

      server->listen(port);
    };

    makeServer(webServers.httpServer, NoSSL, 80);
    makeServer(webServers.httpsServer, UseSSL, 443);
  });

  inet.negotiate_dhcp(3.0,
  [](bool timeout){
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
