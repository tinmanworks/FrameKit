#pragma once
#include <cstdint>
#include <cstddef>

#include "FrameKit/Networking/Socket.hpp"
#include "FrameKit/Networking/Span.hpp"
#include "FrameKit/Networking/NetTypes.hpp"

#if defined(_WIN32)
  #ifndef NOMINMAX
  #define NOMINMAX
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  using fk_native_socket_t = SOCKET;
  static constexpr fk_native_socket_t FK_NATIVE_INVALID = INVALID_SOCKET;
#else
  #include <netinet/in.h>
  using fk_native_socket_t = int;
  static constexpr fk_native_socket_t FK_NATIVE_INVALID = -1;
#endif

namespace FrameKit::Net {

class UdpSocket final : public Socket {
public:
  UdpSocket();
  ~UdpSocket() override;

  NetErr Open();
  NetErr Bind(const IPv4Endpoint& local);

  NetErr SendTo(const IPv4Endpoint& to, ConstByteSpan bytes);
  NetErr RecvFrom(ByteSpan out_buffer, RecvFrom& out);

  SocketKind Kind() const override { return SocketKind::Udp; }
  bool IsOpen() const override;
  NetErr Close() override;

  NetErr SetNonBlocking(bool enabled) override;
  NetErr SetReuseAddr(bool enabled) override;
  NetErr SetBroadcast(bool enabled);

private:
  fk_native_socket_t s_ = FK_NATIVE_INVALID;
};

} // namespace FrameKit::Net
