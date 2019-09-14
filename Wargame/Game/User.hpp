#pragma once

#include "Wargame/Auth/UserAuth.hpp"

namespace Wargame {
  constexpr int newUserIterations = 100'000;

  struct User {
    Auth::UserAuth auth;
  };
}
