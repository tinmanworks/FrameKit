#pragma once
#include "NetTypes.hpp"
#include "Span.hpp"

namespace FrameKit::Net {

// Transport abstraction for datagrams (UDP now, more later).
class IDatagramTransport {
public:
  virtual ~IDatagramTransport() = default;

  virtual bool IsOpen() const = 0;

  virtual NetErr Bind(const IPv4Endpoint& local) = 0;
  virtual NetErr SendTo(const IPv4Endpoint& to, ConstByteSpan bytes) = 0;
  virtual NetErr RecvFrom(ByteSpan out_buffer, RecvFrom& out) = 0;

  virtual NetErr Close() = 0;
};

} // namespace FrameKit::Net
