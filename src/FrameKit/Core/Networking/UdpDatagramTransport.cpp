#include "FrameKit/Networking/UdpDatagramTransport.hpp"
#include "UdpSocket.hpp"
#include <new>

namespace FrameKit::Net {

    struct UdpTransportImpl {
        UdpSocket sock;
    };

    static UdpTransportImpl* I(void* p) { return reinterpret_cast<UdpTransportImpl*>(p); }

    UdpDatagramTransport::UdpDatagramTransport() {
        impl_ = ::operator new(sizeof(UdpTransportImpl), std::nothrow);
        if (impl_) new (impl_) UdpTransportImpl();
    }

    UdpDatagramTransport::~UdpDatagramTransport() {
        if (impl_) {
            I(impl_)->~UdpTransportImpl();
            ::operator delete(impl_);
            impl_ = nullptr;
        }
    }

    NetErr UdpDatagramTransport::Open() {
        if (!impl_) return NetErr::Fail;
        return I(impl_)->sock.Open();
    }

    NetErr UdpDatagramTransport::SetBroadcast(bool enabled) {
        if (!impl_) return NetErr::Fail;
        if (!I(impl_)->sock.IsOpen()) {
            const auto e = Open();
            if (e != NetErr::Ok) return e;
        }
        return I(impl_)->sock.SetBroadcast(enabled);
    }

    NetErr UdpDatagramTransport::SetNonBlocking(bool enabled) {
        if (!impl_) return NetErr::Fail;
        if (!I(impl_)->sock.IsOpen()) {
            const auto e = Open();
            if (e != NetErr::Ok) return e;
        }
        return I(impl_)->sock.SetNonBlocking(enabled);
    }

    NetErr UdpDatagramTransport::SetReuseAddr(bool enabled) {
        if (!impl_) return NetErr::Fail;
        if (!I(impl_)->sock.IsOpen()) {
            const auto e = Open();
            if (e != NetErr::Ok) return e;
        }
        return I(impl_)->sock.SetReuseAddr(enabled);
    }

    NetErr UdpDatagramTransport::SetIPv6Only(bool v6only) {
        if (!impl_) return NetErr::Fail;
        if (!I(impl_)->sock.IsOpen()) {
            const auto e = Open();
            if (e != NetErr::Ok) return e;
        }
        return I(impl_)->sock.SetIPv6Only(v6only);
    }

    bool UdpDatagramTransport::IsOpen() const {
        return impl_ && I(impl_)->sock.IsOpen();
    }

    NetErr UdpDatagramTransport::Bind(const Endpoint& local) {
        if (!impl_) return NetErr::Fail;
        if (!I(impl_)->sock.IsOpen()) {
            const auto e = Open();
            if (e != NetErr::Ok) return e;
        }
        return I(impl_)->sock.Bind(local);
    }

    NetErr UdpDatagramTransport::SendTo(const Endpoint& to, ConstByteSpan bytes) {
        if (!impl_) return NetErr::Fail;
        return I(impl_)->sock.SendTo(to, bytes);
    }

    NetErr UdpDatagramTransport::RecvFrom(ByteSpan out_buffer, RecvFromInfo& out) {
        if (!impl_) return NetErr::Fail;
        return I(impl_)->sock.RecvFrom(out_buffer, out);
    }

    NetErr UdpDatagramTransport::Close() {
        if (!impl_) return NetErr::Ok;
        return I(impl_)->sock.Close();
    }

} // namespace FrameKit::Net
