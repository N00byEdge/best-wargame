#pragma once

#include "Wargame/Auth/Session.hpp"
#include "Wargame/Auth/Passwords.hpp"

namespace Auth {
  struct UserAuth {
    bool authenticate(std::string password) {
      auto attemptedHash = Auth::hashPassword(password, salt, hashIterations);
      return attemptedHash == passwordHash;
    }

    void setPassword(std::string_view const &password, int iterations) {
      passwordHash =
        Auth::hashPassword(password
                         // User always gets a new salt on every password change
                         , salt = Auth::randomizePasswordSalt()
                         , hashIterations = iterations
        );
    }

    PasswordHash passwordHash;
    PasswordSalt salt;
    int hashIterations;
  };
}
