#pragma once
#include "NetTypes.hpp"

namespace FrameKit::Net {

// Abstract base to allow UdpSocket / TcpSocket / others internally.
class Socket {
public:
  virtual ~Socket() = default;

  virtual SocketKind Kind() const = 0;
  virtual bool IsOpen() const = 0;

  virtual NetErr Close() = 0;
  virtual NetErr SetNonBlocking(bool enabled) = 0;
  virtual NetErr SetReuseAddr(bool enabled) = 0;
};

} // namespace FrameKit::Net
