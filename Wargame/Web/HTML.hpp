#pragma once

#include <string>
#include <type_traits>

namespace HTML {
  namespace Impl {
    template<typename ...Args>
    struct PredictedSize;

    template<typename T>
    struct remove_cvref {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;
    };
    
    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;

    template<>
    struct PredictedSize <> {
      std::size_t operator()() const { return 0; }
    };

    template<typename ...Args>
    struct PredictedSize <char, Args...> {
      std::size_t operator()(char const &, Args const &... args) const {
        return 1 + PredictedSize<Args...>{}(args...);
      }
    };

    template<typename ...Args>
    struct PredictedSize <char const *, Args...> {
      std::size_t operator()(char const *const &str, Args const &... args) const {
        return strlen(str) + PredictedSize<Args...>{}(args...);
      }
    };

    template<typename ...Args>
    struct PredictedSize <std::string_view, Args...> {
      std::size_t operator()(std::string_view const &str, Args const &... args) const {
        return str.size() + PredictedSize<Args...>{}(args...);
      }
    };

    template<typename ...Args>
    struct PredictedSize <std::string, Args...> {
      std::size_t operator()(std::string const &str, Args const &... args) const {
        return str.size() + PredictedSize<Args...>{}(args...);
      }
    };

    template<typename ...Args, std::size_t arrSz>
    struct PredictedSize <char [arrSz], Args...> {
      std::size_t operator()(char const (&arr)[arrSz], Args const &... args) const {
        // Check if there is a null terminator in there
        return arrSz - (arr[arrSz - 1] == '\0') + PredictedSize<Args...>{}(args...);
      }
    };

    template<typename CurrArg, typename ...Args>
    void multiAppend(std::string &str, CurrArg &&arg, Args &&... args) {
      str += std::forward<CurrArg>(arg);
      if constexpr(sizeof...(args))
        Impl::multiAppend(str, std::forward<Args>(args)...);
    }
  }

  template<typename ...Args>
  void multiAppend(std::string &str, Args &&... args) {
    str.reserve(str.size() + Impl::PredictedSize<Impl::remove_cvref_t<Args>...>{}(args...));
    Impl::multiAppend(str, std::forward<Args>(args)...);
  }
  
  namespace Impl {
    template<char const *tag>
    struct SimpleTag {
      std::string operator()(std::string &&str) const {
        std::string result;
        multiAppend(result, '<', tag, '>', std::move(str), '<', '/', tag, '>');
        return result;
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
      std::string operator()(std::string &&str, std::string const &arg) const {
        std::string result;
        multiAppend(result, '<', tag, ' ', arg, '>', std::move(str), '<', '/', tag, '>');
        return result;
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

  inline static const auto href =
    [](std::string &&str, char const *target) {
      std::string hrefStr;
      multiAppend(hrefStr, "href=\"", target, '\"');
      return a(std::move(str), std::move(hrefStr));
    };

  inline static const Impl::ArgTag<Impl::form> form;

  inline static const auto formAction =
    [](std::string &&str, char const *target) {
      std::string actionStr;
      multiAppend(actionStr, "action=\"", target, "\" method=\"post\"");
      return form(std::move(str), std::move(actionStr));
    };

  inline static const Impl::ArgTag<Impl::div> div;
  inline static const Impl::ArgTag<Impl::button> button;
  inline static const Impl::ArgTag<Impl::label> label;

  inline static char const br[] = "<br>";
  inline static char const hr[] = "<hr>";

  std::string doctype = "<!DOCTYPE html>";

  std::string errorPage(std::string errorString) {
    std::string responseStr = doctype;
    multiAppend(responseStr, title("Error"), h1(std::move(errorString)));
    return responseStr;
  }

  std::string labelledInputField(std::string &&nameStr
                          , std::string &&labelStr
                          , std::string &&placeholderStr
                          , std::string &&typeStr
                          , bool required) {
    std::string inputStr;
    if(required) {
      HTML::multiAppend(inputStr,
        "<input type=\"", std::move(typeStr),
        "\" placeholder=\"", std::move(placeholderStr),
        "\" name=\"", nameStr, "\" required>"
      );
    } else {
      HTML::multiAppend(inputStr,
        "<input type=\"", std::move(typeStr),
        "\" placeholder=\"", std::move(placeholderStr),
        "\" name=\"", nameStr, "\">"
      );
    }

    std::string field;
    HTML::multiAppend(field,
      label(std::move(labelStr), "for=\"" + std::move(nameStr) + '\"'),
      br,
      std::move(inputStr),
      br
    );
    return field;
  }
}
