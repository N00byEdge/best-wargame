#pragma once

#include <set>
#include <string>
#include <string_view>
#include <type_traits>

#include "Wargame/Web/Network.hpp"
#include "Wargame/Web/HTML.hpp"

namespace Networking {
  constexpr bool debugRouting = false;

  struct Route {
    std::string routeString;
    delegate<void(Request &, Response &)> handler;
  };

  struct CompareRoute {
    template<typename T1, typename T2>
    bool operator()(T1 &&lhs, T2 &&rhs) const {
      auto retval = compareRoutes(cmp(std::forward<T1>(lhs)), cmp(std::forward<T2>(rhs)));
      if constexpr(debugRouting)
        printf("%s\n", retval ? "true" : "false");
      return retval;
    }

    using is_transparent = std::true_type;

  private:
    std::string_view cmp(std::string const &str) const { return str; }
    std::string_view cmp(std::string_view stv) const { return stv; }
    std::string_view cmp(char const *str) const { return str; }
    Route const &cmp(Route const &route) const { return route; }

    bool compareRoutes(Route const &lhs, Route const &rhs) const {
      if constexpr(debugRouting)
        printf("Comparing R\"%s\" < R\"%s\": ", lhs.routeString.c_str(), rhs.routeString.c_str());
      return lhs.routeString < rhs.routeString;
    }

    bool compareRoutes(std::string_view lhs, Route const &route) const {
      if constexpr(debugRouting)
        printf("Comparing \"%s\" < R\"%s\": ", &lhs[0], route.routeString.c_str());
      if(route.routeString.size() >= lhs.size())
        return lhs < route.routeString;
      return lhs.substr(0, route.routeString.size()) < route.routeString;
    }

    bool compareRoutes(Route const &route, std::string_view rhs) const {
      if constexpr(debugRouting)
        printf("Comparing R\"%s\" < \"%s\": ", route.routeString.c_str(), &rhs[0]);
      if(route.routeString.size() >= rhs.size())
        return rhs > route.routeString;

      // Optionally compare an extra character if not '?' to
      // avoid request "abcd" to match route "abc",
      // but "abc?" to still match "abc"
      return rhs.substr(0, route.routeString.size() + (rhs[route.routeString.size()] != '?'))
          > route.routeString;
    }
  };

  struct Router {
    Router(Route &&defaultRoute): defaultRoute{std::move(defaultRoute)} { }

    template<typename T>
    Route const &route(http::Method method, T &&uri) {
      auto methodIt = routes.find(method);
      if(methodIt == routes.end())
        return defaultRoute;

      auto uriIt = methodIt->second.find(std::forward<T>(uri));
      if (uriIt == methodIt->second.end())
        return defaultRoute;

      if constexpr(debugRouting)
        printf("Route match!\n");

      return *uriIt;
    }

    std::map<http::Method, std::set<Route, CompareRoute>> routes;

  private:
    Route const defaultRoute;
  };

  inline static Router router{{"error", // Default route
    [](Networking::Request &, Networking::Response &resp) {
      resp->write_header(http::Not_Found);
      resp->write(HTML::errorPage("Invalid request"));
    }
  }};
}
