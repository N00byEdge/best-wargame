#pragma once

#include <memdisk>
#include <variant>

namespace Filesystem {
  static inline auto initFs = []() {
    fs::memdisk().init_fs([](bool err, auto &){assert(!err);});
    return std::monostate{};
  }();
}
