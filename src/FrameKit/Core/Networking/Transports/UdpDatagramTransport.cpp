#include "FrameKit/Networking/UdpDatagramTransport.hpp"
#include "../Sockets/UdpSocket.hpp"

#include <new>

namespace FrameKit::Net {

struct UdpTransportImpl {
  UdpSocket sock;
  bool open = false;
};

static UdpTransportImpl* impl(void* p) { return reinterpret_cast<UdpTransportImpl*>(p); }

UdpDatagramTransport::UdpDatagramTransport() {
  impl_ = ::operator new(sizeof(UdpTransportImpl), std::nothrow);
  if (impl_) new (impl_) UdpTransportImpl();
}

UdpDatagramTransport::~UdpDatagramTransport() {
  if (impl_) {
    impl(impl_)->~UdpTransportImpl();
    ::operator delete(impl_);
    impl_ = nullptr;
  }
}

NetErr UdpDatagramTransport::Open() {
  if (!impl_) return NetErr::Fail;
  auto* i = impl(impl_);
  const auto e = i->sock.Open();
  if (e != NetErr::Ok) return e;
  i->open = true;
  return NetErr::Ok;
}

NetErr UdpDatagramTransport::SetNonBlocking(bool enabled) {
  if (!impl_) return NetErr::Fail;
  auto* i = impl(impl_);
  if (!i->sock.IsOpen()) {
    const auto e = Open();
    if (e != NetErr::Ok) return e;
  }
  return i->sock.SetNonBlocking(enabled);
}

NetErr UdpDatagramTransport::SetBroadcast(bool enabled) {
  if (!impl_) return NetErr::Fail;
  auto* i = impl(impl_);
  if (!i->sock.IsOpen()) {
    const auto e = Open();
    if (e != NetErr::Ok) return e;
  }
  return i->sock.SetBroadcast(enabled);
}

NetErr UdpDatagramTransport::SetReuseAddr(bool enabled) {
  if (!impl_) return NetErr::Fail;
  auto* i = impl(impl_);
  if (!i->sock.IsOpen()) {
    const auto e = Open();
    if (e != NetErr::Ok) return e;
  }
  return i->sock.SetReuseAddr(enabled);
}

bool UdpDatagramTransport::IsOpen() const {
  return impl_ && impl(impl_)->sock.IsOpen();
}

NetErr UdpDatagramTransport::Bind(const IPv4Endpoint& local) {
  if (!impl_) return NetErr::Fail;
  auto* i = impl(impl_);
  if (!i->sock.IsOpen()) {
    const auto e = Open();
    if (e != NetErr::Ok) return e;
  }
  return i->sock.Bind(local);
}

NetErr UdpDatagramTransport::SendTo(const IPv4Endpoint& to, ConstByteSpan bytes) {
  if (!impl_) return NetErr::Fail;
  return impl(impl_)->sock.SendTo(to, bytes);
}

NetErr UdpDatagramTransport::RecvFrom(ByteSpan out_buffer, RecvFrom& out) {
  if (!impl_) return NetErr::Fail;
  return impl(impl_)->sock.RecvFrom(out_buffer, out);
}

NetErr UdpDatagramTransport::Close() {
  if (!impl_) return NetErr::Ok;
  return impl(impl_)->sock.Close();
}

} // namespace FrameKit::Net
