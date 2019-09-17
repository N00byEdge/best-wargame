#pragma once

#include <variant>

#include "Wargame/Util.hpp"
#include "Wargame/Web/Router.hpp"
#include "Wargame/Game/User.hpp"
#include "Wargame/Web/Middleware.hpp"

namespace Wargame {
  void registrationPage(std::stringstream &stream, Auth::ActiveSession const &session) {
    HTML::makeForm(stream, "/signup", session, [&](){
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
        stream << HTML::hr,
        HTML::button(stream, "type=\"submit\"", "Sign up");
      });
    });
  }

  void homepage(std::stringstream &stream, Auth::ActiveSession const &session) {
    HTML::title(stream, "Best wargame");
    HTML::h1(stream, "Homepage");
    if(session.user.lock()) {
      HTML::href(stream, "/game", "Go to game");
      stream << HTML::br;
      HTML::href(stream, "/logout", "Log out");
    } else {
      HTML::href(stream, "/signup", "Sign up");
      stream << HTML::br;
      HTML::href(stream, "/login", "Login");
    }
  }

  template<typename F>
  void doPage(Networking::Request &request, Networking::Response &response, F &&f) {
    auto &session = *request->get_attribute<SessionAttribute>()->session;
    auto &stream  = request->get_attribute<Wargame::StreamAttribute>()->ss;
    f(stream, session);
  }

  auto addGameRoutes = []() {
    Networking::router.on_get("/",
    [](Networking::Request request, Networking::Response response) {
      doPage(request, response, homepage);
    });

    Networking::router.on_get("/signup",
    [](Networking::Request request, Networking::Response response) {
      doPage(request, response, registrationPage);
    });

    Networking::router.on_post("/signup",
    [](Networking::Request request, Networking::Response response) {
      auto stream = request->get_attribute<Wargame::StreamAttribute>();
      auto uname    = request->source().post_value("username");
      auto pwd      = request->source().post_value("password");
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
