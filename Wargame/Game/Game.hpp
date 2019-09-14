#pragma once

#include <variant>

#include "Wargame/Util.hpp"
#include "Wargame/Web/Router.hpp"
#include "Wargame/Game/User.hpp"

namespace Wargame {
  inline static const std::string registrationPage =
    []() {
      return HTML::doctype + HTML::title("Best wargame - Register") +
        [](){
          std::string formFields;
          HTML::multiAppend(formFields,
            HTML::h1("Registration"),
            HTML::p("Fill in the following fields to complete your registration"),
            HTML::hr,
            HTML::labelledInputField("username", "Username", "Enter username", "text", true),
            HTML::br,
            HTML::labelledInputField("password", "Password", "Enter password", "password", true),
            HTML::br,
            HTML::labelledInputField("password_conf", "Repeat password", "Repeat password", "password", true),
            HTML::br,
            HTML::button("Register", "type=\"submit\"")
          );
          return HTML::formAction(HTML::div(std::move(formFields), ""), "register");
        }();
    }();

  inline static const std::string homepage =
    []() {
      std::string responseStr = HTML::doctype;
      HTML::multiAppend(responseStr,
        HTML::title("Best Wargame"),
        HTML::h1("Homepage"),
        HTML::href("Register", "/register"),
        HTML::br,
        HTML::href("Login", "/login")
      );
      return responseStr;
    }();

  auto addGameRoutes = []() {
    Networking::router.routes[http::GET].emplace(Networking::Route{"/register",
    [](Networking::Request &request, Networking::Response &response) {
      response->write(registrationPage);
    }});

    Networking::router.routes[http::POST].emplace(Networking::Route{"/register",
    [](Networking::Request &request, Networking::Response &response) {
      auto uname = request->post_value("username");
      auto pwd = request->post_value("password");
      auto pwd_conf = request->post_value("password_conf");
      if(pwd != pwd_conf) {
        response->write_header(http::Bad_Request);
        response->write(HTML::errorPage("Passwords do not match!"));
      }
      else {
        Auth::UserAuth auth;
        auth.setPassword(pwd, newUserIterations);
        response->write(
          "Success!\nPassword hash: " + Util::hexstr(auth.passwordHash)
          + "\nPassword salt: " + Util::hexstr(auth.salt)
        );
      }
    }});

    Networking::router.routes[http::GET].emplace(Networking::Route{"/",
    [](Networking::Request &request, Networking::Response &response) {
      response->write(homepage);
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
