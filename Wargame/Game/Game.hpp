#pragma once

#include <variant>

#include "Wargame/Web/Router.hpp"

namespace Wargame {
  auto addGameRoutes = []() {
    Networking::router.routes[http::GET].emplace(Networking::Route{"/",
    [](Networking::Request &request, Networking::Response &response) {
      std::string responseStr = HTML::doctype;
      HTML::multiAppend(responseStr,
        HTML::title("Best Wargame"),
        HTML::h1("Homepage"),
        HTML::a("Register", "href=\"/register\""),
        HTML::br,
        HTML::a("Login", "href=\"/login\"")
      );
      response->write(responseStr);
    }});

    Networking::router.routes[http::GET].emplace(Networking::Route{"/game",
    [](Networking::Request &request, Networking::Response &response) {
      std::string responseStr = HTML::doctype;
      HTML::multiAppend(responseStr,
        HTML::title("Game"),
        HTML::h1("Main game screen")
      );
      response->write(responseStr);
    }});
    return std::monostate{};
  }();
}
