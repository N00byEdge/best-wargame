#pragma once

#include <array>
#include <random>
#include <queue>

#include "Wargame/Web/Network.hpp"
#include "Wargame/Util.hpp"

namespace Wargame {
  struct User;
}

namespace Auth {
  auto sessionTimeoutDuration = std::chrono::hours{24*7}; // 1 week
  using SessionKey = std::array<char, 40>;
  constexpr char sessionChars[] {
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
  };

  SessionKey makeSessionKey() {
    SessionKey key;
    for(std::size_t i = 0; i < key.size(); ++ i)
      std::sample(std::begin(sessionChars), std::end(sessionChars) - 1, key.begin()+ i, 1, Util::rand);
      //                               To exclude null terminator -^
    return key;
  }

  struct UserPtr: std::weak_ptr<Wargame::User> {
    bool operator<(UserPtr const &other) const {
      return owner_before(other);
    }
  };

  struct ActiveSession {
    static constexpr std::size_t SessionIndex = 0;
    static constexpr std::size_t UserIndex = 1;

    SessionKey sessionKey;
    UserPtr user;
    mutable std::chrono::time_point<std::chrono::system_clock> lastActiveAt;
    mutable std::queue<SessionKey> csrfTokens;
  };


  struct CSRFToken {
    static constexpr std::size_t SessionIndex = 0;
    static constexpr std::size_t TokenIndex = 1;

    SessionKey session;
    SessionKey token;
    std::string uri;
  };
}

namespace std {
  // Getter
  template<size_t Ind>
  constexpr auto &get(Auth::ActiveSession &session) noexcept {
    if constexpr(Ind == Auth::ActiveSession::SessionIndex)
      return session.sessionKey;
    else if constexpr(Ind == Auth::ActiveSession::UserIndex)
      return session.user;
  }

  // Const getter
  template<size_t Ind>
  constexpr auto &get(Auth::ActiveSession const &session) noexcept {
    if constexpr(Ind == Auth::ActiveSession::SessionIndex)
      return session.sessionKey;
    else if constexpr(Ind == Auth::ActiveSession::UserIndex)
      return session.user;
  }

  // Getter
  template<size_t Ind>
  constexpr auto &get(Auth::CSRFToken &session) noexcept {
    if constexpr(Ind == Auth::CSRFToken::SessionIndex)
      return session.session;
    else if constexpr(Ind == Auth::CSRFToken::TokenIndex)
      return session.token;
  }

  // Const getter
  template<size_t Ind>
  constexpr auto &get(Auth::CSRFToken const &session) noexcept {
    if constexpr(Ind == Auth::CSRFToken::SessionIndex)
      return session.session;
    else if constexpr(Ind == Auth::CSRFToken::TokenIndex)
      return session.token;
  }
}

template<> struct std::tuple_size<Auth::ActiveSession>: std::integral_constant<std::size_t, 2> { };
template<> struct std::tuple_size<Auth::CSRFToken>: std::integral_constant<std::size_t, 2> { };

#include "CQL/Custom.hpp"

namespace CQL::Custom {
  template<>
  struct Unique<Auth::ActiveSession, Auth::ActiveSession::SessionIndex> {
    constexpr Uniqueness operator()() const {
      return Uniqueness::EnforceUnique;
    }
  };

  template<>
  struct Unique<Auth::ActiveSession, Auth::ActiveSession::UserIndex> {
    constexpr Uniqueness operator()() const {
      return Uniqueness::NotUnique;
    }
  };

  template<>
  struct Unique<Auth::CSRFToken, Auth::CSRFToken::SessionIndex> {
    constexpr Uniqueness operator()() const {
      return Uniqueness::NotUnique;
    }
  };

  template<>
  struct Unique<Auth::CSRFToken, Auth::CSRFToken::TokenIndex> {
    constexpr Uniqueness operator()() const {
      return Uniqueness::AssumeUnique;
    }
  };
}

#include "CQL.hpp"

namespace Auth {
  CQL::Table<ActiveSession> activeSessions;
  CQL::Table<CSRFToken> csrfTokens;

  ActiveSession const *getSession(SessionKey const &sKey) {
    auto session = activeSessions.lookup<ActiveSession::SessionIndex>(sKey);
    if(session) {
      auto timeSinceOnline = std::chrono::system_clock::now() - session->lastActiveAt;
      if(timeSinceOnline > sessionTimeoutDuration)
        activeSessions.erase(std::exchange(session, nullptr));
    }
    return session;
  }

  void updateSession(ActiveSession const *ptr, UserPtr user) {
    activeSessions.update<ActiveSession::UserIndex>(ptr, user);
    ptr->lastActiveAt = std::chrono::system_clock::now();
  }

  void updateSession(ActiveSession const *ptr) {
    updateSession(ptr, ptr->user);
  }

  ActiveSession const *updateSession(SessionKey const &sKey, UserPtr user = {}) {
    auto ptr = getSession(sKey);
    if(!ptr) {
      ptr = activeSessions.emplace(ActiveSession{sKey});
    }

    updateSession(ptr, user);
    return ptr;
  }

  void invalidateCSRFToken(SessionKey const &csrf) {
    auto tok = csrfTokens.lookup<CSRFToken::TokenIndex>(csrf);
    if(tok)
      csrfTokens.erase(tok);
  }

  bool consumeCSRFToken(ActiveSession const &session, SessionKey const &csrf, std::string_view uri) {
    auto tok = csrfTokens.lookup<CSRFToken::TokenIndex>(csrf);
    bool valid = tok && tok->session == session.sessionKey && tok->uri == uri;
    if(valid)
      csrfTokens.erase(tok);
    return valid;
  }

  SessionKey makeCSRFForSession(ActiveSession const &session, std::string_view uri) {
    auto csrf = Auth::makeSessionKey();
    auto token = csrfTokens.emplace(CSRFToken{session.sessionKey, csrf, std::string{uri}});
    session.csrfTokens.emplace(csrf);

    if(session.csrfTokens.size() > 10)
      invalidateCSRFToken(session.csrfTokens.front()), session.csrfTokens.pop();

    return csrf;
  }
}
