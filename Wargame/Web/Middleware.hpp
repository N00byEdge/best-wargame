#pragma once

#include <sstream>

#include <mana/middleware/cookie_parser.hpp>

#include "Wargame/Web/HTML.hpp"
#include "Wargame/Web/Network.hpp"
#include "Wargame/Auth/Session.hpp"
#include "Wargame/Auth/UserAuth.hpp"

namespace Wargame {
  constexpr bool debugMiddleware = false;

  namespace Impl {
    mana::middleware::Cookie_parser cookieParser;
  }

  struct SessionAttribute: mana::Attribute {
    SessionAttribute(Auth::ActiveSession const *sess): session{sess} { }
    Auth::ActiveSession const *session;
  };

  struct StreamAttribute: mana::Attribute {
    StreamAttribute() = default;
    std::stringstream ss;
  };

  template<typename F>
  void middleware(Networking::Request &request, Networking::Response &response, F &&f) {
    Impl::cookieParser.process(request, response, std::make_shared<mana::next_t>([](){}));

    auto stream = std::make_shared<StreamAttribute>();
    request->set_attribute(stream);

    auto sendResponse =
      [&]() {
        response->writer().write(stream->ss.str());
        response->writer().write();
      };
    
    Auth::SessionKey sKey{};
    Auth::ActiveSession const *session = nullptr;
    auto cookies = request->get_attribute<mana::attribute::Cookie_jar>();

    if constexpr(debugMiddleware)
      printf("Cookies parsed!\n");

    if(cookies && cookies->cookie_value("sKey").size() == sKey.size()) {
      auto sKeyStr = cookies->cookie_value("sKey");
      
      if constexpr(debugMiddleware)
        printf("Session key cookie found: %s!\n", sKeyStr.c_str());

      std::copy(sKeyStr.begin(), sKeyStr.end(), sKey.begin());

      session = Auth::getSession(sKey);
      if(!session) {
        if constexpr(debugMiddleware)
          printf("Creating new session!\n");
        session = Auth::updateSession(sKey = Auth::makeSessionKey());
        response->cookie(http::Cookie{"sKey", {sKey.begin(), sKey.end()}});
      }
      else {
        if constexpr(debugMiddleware)
          printf("Updating session!\n");
        Auth::updateSession(session);
      }
    } else {
      if constexpr(debugMiddleware)
        printf("Created and sending new session key.\n");

      sKey = Auth::makeSessionKey();
      response->cookie(http::Cookie{"sKey", {sKey.begin(), sKey.end()}});
      session = Auth::updateSession(sKey);
    }
    
    if constexpr(debugMiddleware)
      printf("Setting session attribute.\n");

    request->set_attribute(std::make_shared<SessionAttribute>(session));
    stream->ss << HTML::doctype << R"(<meta charset="utf-8"/>)";

    if(request->method() == http::POST) {
      Auth::SessionKey csrf;
      auto csrfString = request->source().post_value("csrfToken");
      if(csrfString.size() != csrf.size())
        return;
      std::copy(csrfString.begin(), csrfString.end(), csrf.begin());
      auto uri = request->uri().path();
      if(!Auth::consumeCSRFToken(*session, csrf, uri)) {
        // Someone attempted CSRF; deny the POST.
        HTML::errorPage(stream->ss, "Invalid CSRF token, try sending your request again.");
        sendResponse();
        return;
      }
    }

    f();

    sendResponse();
  }
}
