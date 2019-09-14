#pragma once

#include <http>
#include <https>
#include <net/interfaces>

namespace Networking {
  using Response = http::Response_writer_ptr;
  using Request = http::Request_ptr;
}
