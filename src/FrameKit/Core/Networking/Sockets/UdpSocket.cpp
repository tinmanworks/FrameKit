#include "UdpSocket.hpp"
#include "../Platform/PlatformNetInit.hpp"

#include <cstring>

#if !defined(_WIN32)
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/socket.h>
  #include <errno.h>
#else
  #include <ws2tcpip.h>
#endif

namespace FrameKit::Net {

static sockaddr_in to_sockaddr(const IPv4Endpoint& ep) {
  sockaddr_in a{};
  a.sin_family = AF_INET;
  a.sin_port = htons(ep.port_host);
  a.sin_addr.s_addr = htonl(ep.addr_host);
  return a;
}

UdpSocket::UdpSocket() { Detail::PlatformNetAcquire(); }
UdpSocket::~UdpSocket() { (void)Close(); Detail::PlatformNetRelease(); }

bool UdpSocket::IsOpen() const {
#if defined(_WIN32)
  return s_ != INVALID_SOCKET;
#else
  return s_ >= 0;
#endif
}

NetErr UdpSocket::Open() {
  if (IsOpen()) return NetErr::Ok;
  s_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (!IsOpen()) return NetErr::Fail;
  return NetErr::Ok;
}

NetErr UdpSocket::Bind(const IPv4Endpoint& local) {
  if (!IsOpen()) {
    const auto e = Open();
    if (e != NetErr::Ok) return e;
  }
  const auto a = to_sockaddr(local);
  if (::bind(s_, (const sockaddr*)&a, sizeof(a)) != 0) return NetErr::Fail;
  return NetErr::Ok;
}

NetErr UdpSocket::SendTo(const IPv4Endpoint& to, ConstByteSpan bytes) {
  if (!IsOpen()) return NetErr::NotOpen;
  const auto a = to_sockaddr(to);
#if defined(_WIN32)
  const int sent = ::sendto(s_, (const char*)bytes.data, (int)bytes.size, 0, (const sockaddr*)&a, sizeof(a));
  if (sent == SOCKET_ERROR) return NetErr::Fail;
  return ((size_t)sent == bytes.size) ? NetErr::Ok : NetErr::Fail;
#else
  const ssize_t sent = ::sendto(s_, bytes.data, bytes.size, 0, (const sockaddr*)&a, sizeof(a));
  if (sent < 0) return NetErr::Fail;
  return ((size_t)sent == bytes.size) ? NetErr::Ok : NetErr::Fail;
#endif
}

NetErr UdpSocket::RecvFrom(ByteSpan out_buffer, RecvFrom& out) {
  if (!IsOpen()) return NetErr::NotOpen;

  sockaddr_in from{};
#if defined(_WIN32)
  int fromlen = sizeof(from);
  const int recvd = ::recvfrom(s_, (char*)out_buffer.data, (int)out_buffer.size, 0, (sockaddr*)&from, &fromlen);
  if (recvd == SOCKET_ERROR) {
    const int err = WSAGetLastError();
    if (err == WSAEWOULDBLOCK) return NetErr::WouldBlock;
    return NetErr::Fail;
  }
#else
  socklen_t fromlen = sizeof(from);
  const ssize_t recvd = ::recvfrom(s_, out_buffer.data, out_buffer.size, 0, (sockaddr*)&from, &fromlen);
  if (recvd < 0) {
    if (errno == EWOULDBLOCK || errno == EAGAIN) return NetErr::WouldBlock;
    return NetErr::Fail;
  }
#endif

  out.bytes = (size_t)recvd;
  out.from.addr_host = ntohl(from.sin_addr.s_addr);
  out.from.port_host = ntohs(from.sin_port);
  return NetErr::Ok;
}

NetErr UdpSocket::Close() {
  if (!IsOpen()) return NetErr::Ok;
#if defined(_WIN32)
  ::closesocket(s_);
  s_ = INVALID_SOCKET;
#else
  ::close(s_);
  s_ = -1;
#endif
  return NetErr::Ok;
}

NetErr UdpSocket::SetNonBlocking(bool enabled) {
  if (!IsOpen()) return NetErr::NotOpen;
#if defined(_WIN32)
  u_long nb = enabled ? 1UL : 0UL;
  if (::ioctlsocket(s_, FIONBIO, &nb) != 0) return NetErr::Fail;
  return NetErr::Ok;
#else
  int flags = ::fcntl(s_, F_GETFL, 0);
  if (flags < 0) return NetErr::Fail;
  if (enabled) flags |= O_NONBLOCK;
  else flags &= ~O_NONBLOCK;
  if (::fcntl(s_, F_SETFL, flags) != 0) return NetErr::Fail;
  return NetErr::Ok;
#endif
}

NetErr UdpSocket::SetReuseAddr(bool enabled) {
  if (!IsOpen()) return NetErr::NotOpen;
  int v = enabled ? 1 : 0;
  if (::setsockopt(s_, SOL_SOCKET, SO_REUSEADDR, (const char*)&v, sizeof(v)) != 0) return NetErr::Fail;
  return NetErr::Ok;
}

NetErr UdpSocket::SetBroadcast(bool enabled) {
  if (!IsOpen()) return NetErr::NotOpen;
  int v = enabled ? 1 : 0;
  if (::setsockopt(s_, SOL_SOCKET, SO_BROADCAST, (const char*)&v, sizeof(v)) != 0) return NetErr::Fail;
  return NetErr::Ok;
}

} // namespace FrameKit::Net
