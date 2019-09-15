#pragma once

#include <array>
#include <random>

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

  constexpr std::size_t SessionIndex = 0;
  constexpr std::size_t UserIndex = 1;

  struct UserPtr: std::weak_ptr<Wargame::User> {
    bool operator<(UserPtr const &other) const {
      return owner_before(other);
    }
  };

  struct ActiveSession {
    SessionKey sessionKey;
    UserPtr user;
    mutable std::chrono::time_point<std::chrono::system_clock> lastActiveAt;
    mutable std::optional<SessionKey> currentCSRF;
  };
}

namespace std {
  // Getter
  template<size_t Ind>
  constexpr auto &get(Auth::ActiveSession &session) noexcept {
    if constexpr(Ind == Auth::SessionIndex)
      return session.sessionKey;
    else if constexpr(Ind == Auth::UserIndex)
      return session.user;
  }

  // Const getter
  template<size_t Ind>
  constexpr auto &get(Auth::ActiveSession const &session) noexcept {
    if constexpr(Ind == Auth::SessionIndex)
      return session.sessionKey;
    else if constexpr(Ind == Auth::UserIndex)
      return session.user;
  }
}

template<> struct std::tuple_size<Auth::ActiveSession>: std::integral_constant<std::size_t, 2> { };

#include "CQL/Custom.hpp"

namespace CQL::Custom {
  template<>
  struct Unique<Auth::ActiveSession, Auth::SessionIndex> {
    constexpr Uniqueness operator()() const {
      return Uniqueness::EnforceUnique;
    }
  };

  template<>
  struct Unique<Auth::ActiveSession, Auth::UserIndex> {
    constexpr Uniqueness operator()() const {
      return Uniqueness::NotUnique;
    }
  };
}

#include "CQL.hpp"

namespace Auth {
  CQL::Table<ActiveSession> activeSessions;

  ActiveSession const *getSession(SessionKey const &sKey) {
    auto session = activeSessions.lookup<SessionIndex>(sKey);
    if(session) {
      auto timeSinceOnline = std::chrono::system_clock::now() - session->lastActiveAt;
      if(timeSinceOnline > sessionTimeoutDuration)
        activeSessions.erase(std::exchange(session, nullptr));
    }
    return session;
  }

  void updateSession(ActiveSession const *ptr, UserPtr user) {
    activeSessions.update<UserIndex>(ptr, user);
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
}
