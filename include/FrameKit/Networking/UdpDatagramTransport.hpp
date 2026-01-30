#pragma once
#include "DatagramTransport.hpp"

namespace FrameKit::Net {

// Forward-declared public type to let apps construct it if they want,
// but implementation is in src/Core/FrameKit/Networking.
class UdpDatagramTransport final : public IDatagramTransport {
public:
  UdpDatagramTransport();
  ~UdpDatagramTransport() override;

  // Optional configuration
  NetErr Open();
  NetErr SetNonBlocking(bool enabled);
  NetErr SetBroadcast(bool enabled);
  NetErr SetReuseAddr(bool enabled);

  bool IsOpen() const override;

  NetErr Bind(const IPv4Endpoint& local) override;
  NetErr SendTo(const IPv4Endpoint& to, ConstByteSpan bytes) override;
  NetErr RecvFrom(ByteSpan out_buffer, RecvFrom& out) override;
  NetErr Close() override;

private:
  void* impl_ = nullptr; // pImpl to avoid exposing platform headers in public includes
};

} // namespace FrameKit::Net
