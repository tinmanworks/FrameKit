#pragma once
#include <cstdint>
#include <cstddef>

namespace FrameKit::Net {

struct IPv4Endpoint {
  uint32_t addr_host = 0; // host order, e.g. 0x7F000001
  uint16_t port_host = 0; // host order
};

struct RecvFrom {
  IPv4Endpoint from{};
  size_t bytes = 0;
};

enum class NetErr : int32_t {
  Ok = 0,
  Fail = -1,
  WouldBlock = -2,
  InvalidArg = -3,
  NotOpen = -4,
  TooLarge = -5,
  NoCapacity = -6,
};

enum class SocketKind : uint8_t {
  Unknown = 0,
  Udp = 1,
  Tcp = 2, // future
};

enum class WireMode : uint8_t {
  Raw = 0,    // [app_header][payload]
  Framed = 1, // [preheader][app_header][payload]
};

} // namespace FrameKit::Net
