#pragma once

#include <variant>

#include "Wargame/Util.hpp"
#include "Wargame/Web/Router.hpp"
#include "Wargame/Game/User.hpp"
#include "Wargame/Web/Middleware.hpp"

namespace Wargame {
  void registrationPage(std::stringstream &stream, std::string const &csrfToken) {
    HTML::formAction(stream, "/signup", [&](){
      HTML::title(stream, "Best wargame - Sign up");
      HTML::div(stream, "", [&]() {
        HTML::h1(stream, "Registration");
        HTML::p (stream, "Fill in the following fields to complete your registration");
        stream << HTML::hr;
        HTML::labelledInputField(stream, "username", "Username", "Enter username", "text", true);
        stream << HTML::br;
        HTML::labelledInputField(stream, "password", "Password", "Enter password", "password", true);
        stream << HTML::br;
        HTML::labelledInputField(stream, "password_conf", "Repeat password", "Repeat password", "password", true);
        stream << HTML::br;
        HTML::csrfField(stream, csrfToken);
        stream << HTML::hr,
        HTML::button(stream, "type=\"submit\"", "Sign up");
      });
    });
  }

  inline static const std::string homepage =
    []() {
      std::stringstream stream;
      HTML::title(stream, "Best wargame");
      HTML::h1(stream, "Homepage");
      HTML::href(stream, "/signup", "Sign up");
      stream << HTML::br;
      HTML::href(stream, "/login", "Login");
      return stream.str();
    }();

  auto addGameRoutes = []() {
    Networking::router.on_get("/",
    [](Networking::Request request, Networking::Response response) {
      request->get_attribute<Wargame::StreamAttribute>()->ss << homepage;
    });

    Networking::router.on_get("/signup",
    [](Networking::Request request, Networking::Response response) {
      auto session = request->get_attribute<SessionAttribute>();
      auto csrf = Auth::makeSessionKey();
      session->session->currentCSRF = csrf;
      registrationPage(request->get_attribute<Wargame::StreamAttribute>()->ss, {csrf.begin(), csrf.end()});
    });

    Networking::router.on_post("/signup",
    [](Networking::Request request, Networking::Response response) {
      auto stream = request->get_attribute<Wargame::StreamAttribute>();
      auto uname = request->source().post_value("username");
      auto pwd = request->source().post_value("password");
      auto pwd_conf = request->source().post_value("password_conf");

      if(pwd != pwd_conf) {
        HTML::errorPage(stream->ss, "Passwords do not match!");
        return;
      }

      if(pwd.empty()) {
        HTML::errorPage(stream->ss, "Password cannot be empty!");
        return;
      }

      Auth::UserAuth auth;
      auth.setPassword(pwd, newUserIterations);
      fmt::print(stream->ss,
        "Success!<br>"
        "Password hash: <pre>{}</pre><br>"
        "Password salt: <pre>{}</pre><br>"
        ,
        Util::hexstr(auth.passwordHash),
        Util::hexstr(auth.salt)
      );
    });

    return std::monostate{};
  }();
}
