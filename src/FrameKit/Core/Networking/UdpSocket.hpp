#pragma once
#include "FrameKit/Networking/Networking.hpp"
#include "PlatformNetInit.hpp"

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
		NetErr Bind(const Endpoint& local);

		NetErr SendTo(const Endpoint& to, ConstByteSpan bytes);
		NetErr RecvFrom(ByteSpan out_buffer, RecvFromInfo& out);

		SocketKind Kind() const override { return SocketKind::Udp; }
		bool IsOpen() const override;
		NetErr Close() override;

		NetErr SetBroadcast(bool enabled);
		NetErr SetNonBlocking(bool enabled) override;
		NetErr SetReuseAddr(bool enabled) override;

		NetErr SetIPv6Only(bool v6only);

	private:
		fk_native_socket_t s_ = FK_NATIVE_INVALID;
	};

} // namespace FrameKit::Net
