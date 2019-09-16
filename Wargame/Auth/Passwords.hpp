#pragma once

#include <array>
#include <random>
#include <string>
#include <openssl/sha.h>

#include "Wargame/Util.hpp"

namespace Auth {
  using PasswordSalt = std::array<unsigned char, SHA512_DIGEST_LENGTH>;
  using PasswordHash = std::array<unsigned char, SHA512_DIGEST_LENGTH>;

  PasswordHash hashPassword(std::string_view const &password, PasswordSalt const &salt, int iterations) {
    SHA512_CTX ctx;
    PasswordHash out;

    SHA512_Init(&ctx);
    SHA512_Update(&ctx, salt.data(), salt.size());
    SHA512_Update(&ctx, password.data(), password.size());
    SHA512_Final(out.data(), &ctx);

    for(int i = 0; i < iterations; ++ i)
      SHA512(out.data(), out.size(), out.data());

    return out;
  }

  PasswordSalt randomizePasswordSalt() {
    PasswordSalt out;
    for(auto &c: out)
      c = Util::rand();
    return out;
  }
}
