#pragma once

#include <array>
#include <random>

namespace Auth {
  using SessionKey = std::array<char, 40>;
  constexpr char sessionChars[] {
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890!#%&/()=?^'*-_.:,;[]{}$~"
  };

  namespace Impl {
    inline static std::random_device rand{};
  }

  SessionKey makeSessionKey() {
    SessionKey key;
    std::sample(std::begin(sessionChars), std::end(sessionChars), key.begin(), key.size(), Impl::rand);
    return key;
  }
}
