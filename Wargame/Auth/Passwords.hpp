#pragma once

#include <array>
#include <random>
#include <string>
#include <openssl/sha.h>

namespace Auth {
  using PasswordSalt = std::array<unsigned char, 0x20>;
  using PasswordHash = std::array<unsigned char, SHA512_DIGEST_LENGTH>;

  namespace Impl {
    inline static std::random_device rand{};
  }

  PasswordHash hashPassword(std::string password, PasswordSalt const &salt, int iterations) {
    PasswordHash out;
    password.insert(password.begin(), salt.begin(), salt.end());
    SHA512(reinterpret_cast<unsigned char const *>(password.c_str()), password.size(), out.data());
    for(int i = 0; i < iterations; ++ i) {
      SHA512(out.data(), out.size(), out.data());
    }
    return out;
  }

  PasswordSalt randomizePasswordSalt() {
    PasswordSalt out;
    for(auto &c: out)
      c = Impl::rand();
    return out;
  }
}
