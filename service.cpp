#include <os>
#include <https>
#include <memdisk>

#include <net/interfaces>

#include <mana/middleware/parsley.hpp>
#include <mana/middleware/director.hpp>
#include <mana/middleware/butler.hpp>
#include <mana/middleware/cookie_parser.hpp>

#include <mana/server.hpp>

namespace {
  auto &inet = net::Interfaces::get(0);

  using WebServer = std::unique_ptr<http::Server>;
  struct {
    WebServer httpServer, httpsServer;
  } webServers;

  using Response = http::Response_writer_ptr;
  using Request = http::Request_ptr;

  using Session = std::array<char, 20>;

  struct UserAccount {
    std::string username;
  };

  std::map<Session, UserAccount> activeSessions;
}

void makeResponse(Request &request, Response &response) {
  response->write_header(http::OK);
  response->write("Hello what's up");
}

void Service::start() {
  fs::memdisk().init_fs([](bool err, auto &){assert(!err);});
  inet.on_config([](net::Inet::Stack &) {
    auto makeServer = [](WebServer &server, auto ssl, int port) {
      if constexpr(ssl) {
        server = std::make_unique<http::OpenSSL_server>("/ssl.pem", "/ssl.key", inet.tcp());
      } else {
        server = std::make_unique<http::Server>(inet.tcp());
      }

      server->on_request([](Request request, Response response){
        makeResponse(request, response);
        response->write();
      });

      server->listen(port);
    };

    makeServer(webServers.httpServer, std::false_type{}, 80);
    makeServer(webServers.httpsServer, std::true_type{}, 443);
  });

	inet.negotiate_dhcp(3.0, [](bool timeout){
    if(timeout) {
      printf("DHCP negotiation failed, falling back to static assignment.");
      inet.network_config(
        {10,0,0,4},
        {255,255,255,0},
        {10,0,0,1},
        {1,1,1,1}
      );
    }
  });
}
