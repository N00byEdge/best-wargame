#pragma once

#include <array>
#include <random>

#include "Wargame/Util.hpp"

namespace Auth {
  using SessionKey = std::array<char, 40>;
  constexpr char sessionChars[] {
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890!#%&/()=?^'*-_.:,;[]{}$~"
  };

  SessionKey makeSessionKey() {
    SessionKey key;
    std::sample(std::begin(sessionChars), std::end(sessionChars), key.begin(), key.size(), Util::rand);
    return key;
  }
}
