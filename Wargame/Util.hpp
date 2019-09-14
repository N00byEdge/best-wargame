#pragma once

#include <array>
#include <random>

namespace Util {
  inline static std::random_device rand{};

  template<std::size_t sz>
  std::string hexstr(std::array<unsigned char, sz> const &arr) {
    auto hex = [](char c) {
      return "0123456789abcdef"[c & 0xf];
    };
    
    std::string result;
    for(auto &v: arr) {
      result += hex(v >> 4);
      result += hex(v);
    }
    return result;
  }
}
