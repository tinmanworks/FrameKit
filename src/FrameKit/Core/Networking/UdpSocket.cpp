#include "UdpSocket.hpp"
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

    static void make_v4_mapped(uint8_t out16[16], uint32_t v4_host) {
        std::memset(out16, 0, 16);
        out16[10] = 0xFF;
        out16[11] = 0xFF;
        const uint32_t v4_net = htonl(v4_host);
        std::memcpy(out16 + 12, &v4_net, 4);
    }

    static bool is_v4_mapped(const uint8_t in16[16]) {
        for (int i = 0; i < 10; ++i) if (in16[i] != 0) return false;
        return in16[10] == 0xFF && in16[11] == 0xFF;
    }

    static uint32_t extract_v4_from_mapped_host(const uint8_t in16[16]) {
        uint32_t v4_net = 0;
        std::memcpy(&v4_net, in16 + 12, 4);
        return ntohl(v4_net);
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

        s_ = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (!IsOpen()) return NetErr::Fail;

        (void)SetIPv6Only(false);
        return NetErr::Ok;
    }

    NetErr UdpSocket::SetIPv6Only(bool v6only) {
        if (!IsOpen()) return NetErr::NotOpen;
        int v = v6only ? 1 : 0;
        if (::setsockopt(s_, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&v, sizeof(v)) != 0) return NetErr::Fail;
        return NetErr::Ok;
    }

    NetErr UdpSocket::Bind(const Endpoint& local) {
        if (!IsOpen()) {
            const auto e = Open();
            if (e != NetErr::Ok) return e;
        }

        sockaddr_in6 a{};
        a.sin6_family = AF_INET6;

        if (local.family == AddressFamily::IPv4) {
            a.sin6_port = htons(local.v4.port_host);
            uint8_t mapped[16];
            make_v4_mapped(mapped, local.v4.addr_host);
            std::memcpy(&a.sin6_addr, mapped, 16);
            a.sin6_scope_id = 0;
        }
        else {
            a.sin6_port = htons(local.v6.port_host);
            std::memcpy(&a.sin6_addr, local.v6.addr, 16);
            a.sin6_scope_id = local.v6.scope_id;
        }

        if (::bind(s_, (const sockaddr*)&a, sizeof(a)) != 0) return NetErr::Fail;
        return NetErr::Ok;
    }

    NetErr UdpSocket::SendTo(const Endpoint& to, ConstByteSpan bytes) {
        if (!IsOpen()) return NetErr::NotOpen;

        sockaddr_in6 a{};
        a.sin6_family = AF_INET6;

        if (to.family == AddressFamily::IPv4) {
            a.sin6_port = htons(to.v4.port_host);
            uint8_t mapped[16];
            make_v4_mapped(mapped, to.v4.addr_host);
            std::memcpy(&a.sin6_addr, mapped, 16);
            a.sin6_scope_id = 0;
        }
        else {
            a.sin6_port = htons(to.v6.port_host);
            std::memcpy(&a.sin6_addr, to.v6.addr, 16);
            a.sin6_scope_id = to.v6.scope_id;
        }

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

    NetErr UdpSocket::RecvFrom(ByteSpan out_buffer, RecvFromInfo& out) {
        if (!IsOpen()) return NetErr::NotOpen;

        sockaddr_in6 from{};
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

        uint8_t addr16[16];
        std::memcpy(addr16, &from.sin6_addr, 16);

        if (is_v4_mapped(addr16)) {
            const uint32_t v4_host = extract_v4_from_mapped_host(addr16);
            const uint16_t port_host = ntohs(from.sin6_port);
            out.from = Endpoint::FromV4(v4_host, port_host);
        }
        else {
            const uint16_t port_host = ntohs(from.sin6_port);
            out.from = Endpoint::FromV6(addr16, port_host, from.sin6_scope_id);
        }

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

    NetErr UdpSocket::SetBroadcast(bool enabled) {
        if (!IsOpen()) return NetErr::NotOpen;

#if defined(_WIN32)
        BOOL opt = enabled ? TRUE : FALSE;
        const int r = ::setsockopt(
            s_,
            SOL_SOCKET,
            SO_BROADCAST,
            reinterpret_cast<const char*>(&opt),
            sizeof(opt)
        );
        if (r != 0) return NetErr::Fail;
#else
        int opt = enabled ? 1 : 0;
        const int r = ::setsockopt(
            s_,
            SOL_SOCKET,
            SO_BROADCAST,
            &opt,
            sizeof(opt)
        );
        if (r != 0) return NetErr::Fail;
#endif

        return NetErr::Ok;
    }

    NetErr UdpSocket::SetNonBlocking(bool enabled) {
        if (!IsOpen()) return NetErr::NotOpen;
#if defined(_WIN32)
        u_long nb = enabled ? 1UL : 0UL;
        return (::ioctlsocket(s_, FIONBIO, &nb) == 0) ? NetErr::Ok : NetErr::Fail;
#else
        int flags = ::fcntl(s_, F_GETFL, 0);
        if (flags < 0) return NetErr::Fail;
        if (enabled) flags |= O_NONBLOCK;
        else flags &= ~O_NONBLOCK;
        return (::fcntl(s_, F_SETFL, flags) == 0) ? NetErr::Ok : NetErr::Fail;
#endif
    }

    NetErr UdpSocket::SetReuseAddr(bool enabled) {
        if (!IsOpen()) return NetErr::NotOpen;
        int v = enabled ? 1 : 0;
        return (::setsockopt(s_, SOL_SOCKET, SO_REUSEADDR, (const char*)&v, sizeof(v)) == 0) ? NetErr::Ok : NetErr::Fail;
    }

} // namespace FrameKit::Net
