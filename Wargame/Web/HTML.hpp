#pragma once

#include <string>
#include <sstream>
#include <type_traits>

#define FMT_HEADER_ONLY
#include <fmt/ostream.h>

namespace HTML {
  namespace Impl {
    template<char const *tag>
    struct SimpleTag {
      void operator()(std::stringstream &stream,  std::string_view content) const {
        fmt::print(stream,
          "<{0}>{1}</{0}>",
          tag, content
        );
      }

      template<typename F>
      std::enable_if_t<!std::is_convertible_v<F, std::string>, void>
      operator()(std::stringstream &stream, F &&f) const {
        fmt::print(stream, "<{}>", tag);
        f();
        fmt::print(stream, "</{}>", tag);
      }
    };

    inline static char const h1[] = "h1";
    inline static char const h2[] = "h2";
    inline static char const h3[] = "h3";
    inline static char const h4[] = "h4";

    inline static char const title[] = "title";
    inline static char const p[] = "p";
    inline static char const b[] = "b";

    template<char const *tag>
    struct ArgTag {
      void operator()(std::stringstream &stream, std::string const &arg, std::string_view content) const {
        fmt::print(stream,
          "<{0}{1}>{2}</{0}>",
          tag, arg.empty() ? "" : ' ' + arg, content
        );
      }

      template<typename F>
      std::enable_if_t<!std::is_convertible_v<F, std::string>, void>
      operator()(std::stringstream &stream, std::string const &arg, F &&f) const {
        fmt::print(stream, "<{}{}>", tag, arg.empty() ? "" : ' ' + arg);
        f();
        fmt::print(stream, "</{}>", tag);
      }
    };

    inline static char const a[] = "a";
    inline static char const form[] = "form";
    inline static char const div[] = "div";
    inline static char const button[] = "button";
    inline static char const label[] = "label";
  }

  inline static const Impl::SimpleTag<Impl::h1> h1;
  inline static const Impl::SimpleTag<Impl::h2> h2;
  inline static const Impl::SimpleTag<Impl::h3> h3;
  inline static const Impl::SimpleTag<Impl::h4> h4;

  inline static const Impl::SimpleTag<Impl::title> title;
  inline static const Impl::SimpleTag<Impl::p> p;
  inline static const Impl::SimpleTag<Impl::b> b;

  inline static const Impl::ArgTag<Impl::a> a;

  template<typename F>
  void href(std::stringstream &stream, std::string const &target, F &&f) {
    a(stream, "href=\"" + target + '\"', std::forward<F>(f));
  };

  inline static const Impl::ArgTag<Impl::form> form;

  template<typename F>
  void formAction(std::stringstream &stream, std::string const &target, F &&f) {
    // @TODO: Move the CSRF token things to over here
    form(stream, "action=\"" + target + "\" method=\"post\"", std::forward<F>(f));
  };

  inline static const Impl::ArgTag<Impl::div> div;
  inline static const Impl::ArgTag<Impl::button> button;
  inline static const Impl::ArgTag<Impl::label> label;

  inline static char const br[] = "<br>";
  inline static char const hr[] = "<hr>";

  std::string doctype = "<!DOCTYPE html>";

  void errorPage(std::stringstream &stream, std::string const &errorString) {
    title(stream, "Error");
    h1(stream, errorString);
  }

  void labelledInputField(std::stringstream &stream
                        , std::string &&nameStr
                        , std::string &&labelStr
                        , std::string &&placeholderStr
                        , std::string &&typeStr
                        , bool required) {
    fmt::print(stream,
      R"(<label for="{}">{}</label><br>)"
      R"(<input type="{}" placeholder="{}" name="{}"{}>)",
      nameStr, std::move(labelStr),
      std::move(typeStr), std::move(placeholderStr), nameStr, required ? " required" : ""
    );
  }

  void csrfField(std::stringstream &stream, std::string const &csrfToken) {
    fmt::print(stream, R"(<input type="hidden" name="csrfToken" value="{}">)", csrfToken);
  }
}
