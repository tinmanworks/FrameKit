#include "FrameKit/Networking/UdpSocket.h"

#include <cstring>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
static constexpr socket_t kInvalidSocket = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
static constexpr socket_t kInvalidSocket = -1;
#endif

namespace FrameKit::Net {

    static socket_t& as_sock(void* p) {
        return *reinterpret_cast<socket_t*>(&p);
    }
    static socket_t sock_val(void* p) {
        return reinterpret_cast<socket_t>(p);
    }
    static void set_sock(void*& p, socket_t s) {
        p = reinterpret_cast<void*>(s);
    }

    static void close_socket(socket_t s) {
#if defined(_WIN32)
        closesocket(s);
#else
        ::close(s);
#endif
    }

    UdpSocket::UdpSocket() : m_Handle(nullptr) {
        set_sock(m_Handle, kInvalidSocket);
    }

    UdpSocket::~UdpSocket() {
        Close();
    }

    bool UdpSocket::Open() {
        if (IsOpen()) return true;

        socket_t s = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (s == kInvalidSocket) return false;

        set_sock(m_Handle, s);
        SetNonBlocking();

        // reuse addr for quick restart
        int yes = 1;
#if defined(_WIN32)
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
#else
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif
        return true;
    }

    void UdpSocket::Close() {
        if (!IsOpen()) return;
        close_socket(sock_val(m_Handle));
        set_sock(m_Handle, kInvalidSocket);
    }

    bool UdpSocket::IsOpen() const {
        return sock_val(m_Handle) != kInvalidSocket;
    }

    void UdpSocket::SetNonBlocking() {
        socket_t s = sock_val(m_Handle);
#if defined(_WIN32)
        u_long mode = 1;
        ioctlsocket(s, FIONBIO, &mode);
#else
        int flags = fcntl(s, F_GETFL, 0);
        if (flags >= 0) fcntl(s, F_SETFL, flags | O_NONBLOCK);
#endif
    }

    bool UdpSocket::Bind(uint16_t port) {
        if (!IsOpen() && !Open()) return false;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        socket_t s = sock_val(m_Handle);
        if (::bind(s, (sockaddr*)&addr, sizeof(addr)) != 0) {
            return false;
        }
        return true;
    }

    bool UdpSocket::SetBroadcast(bool enabled) {
        if (!IsOpen() && !Open()) return false;
        int opt = enabled ? 1 : 0;
        socket_t s = sock_val(m_Handle);
#if defined(_WIN32)
        return setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char*)&opt, sizeof(opt)) == 0;
#else
        return setsockopt(s, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) == 0;
#endif
    }

    bool UdpSocket::SendTo(const Endpoint& to, std::span<const uint8_t> bytes) {
        if (!to.IsValid()) return false;
        if (!IsOpen() && !Open()) return false;

        sockaddr_in dst{};
        dst.sin_family = AF_INET;
        dst.sin_port = htons(to.port);

#if defined(_WIN32)
        if (InetPtonA(AF_INET, to.ip.c_str(), &dst.sin_addr) != 1) return false;
#else
        if (inet_pton(AF_INET, to.ip.c_str(), &dst.sin_addr) != 1) return false;
#endif

        socket_t s = sock_val(m_Handle);
#if defined(_WIN32)
        int sent = ::sendto(s, (const char*)bytes.data(), (int)bytes.size(), 0, (sockaddr*)&dst, (int)sizeof(dst));
        if (sent == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) return false;
            return false;
        }
#else
        int sent = ::sendto(s, bytes.data(), bytes.size(), 0, (sockaddr*)&dst, (socklen_t)sizeof(dst));
        if (sent < 0) return false;
#endif
        return true;
    }

    std::optional<UdpSocket::RecvPacket> UdpSocket::RecvOnce(size_t maxBytes) {
        if (!IsOpen()) return std::nullopt;

        RecvPacket pkt;
        pkt.data.resize(maxBytes);

        sockaddr_in from{};
#if defined(_WIN32)
        int fromLen = (int)sizeof(from);
        int n = ::recvfrom(sock_val(m_Handle), (char*)pkt.data.data(), (int)pkt.data.size(), 0, (sockaddr*)&from, &fromLen);
        if (n == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) return std::nullopt;
            return std::nullopt;
        }
#else
        socklen_t fromLen = (socklen_t)sizeof(from);
        int n = ::recvfrom(sock_val(m_Handle), pkt.data.data(), pkt.data.size(), 0, (sockaddr*)&from, &fromLen);
        if (n < 0) return std::nullopt;
#endif

        pkt.data.resize((size_t)n);

        char ipbuf[64]{};
#if defined(_WIN32)
        InetNtopA(AF_INET, &from.sin_addr, ipbuf, sizeof(ipbuf));
#else
        inet_ntop(AF_INET, &from.sin_addr, ipbuf, sizeof(ipbuf));
#endif
        pkt.from.ip = ipbuf;
        pkt.from.port = ntohs(from.sin_port);

        return pkt;
    }

} // namespace FrameKit::Net
