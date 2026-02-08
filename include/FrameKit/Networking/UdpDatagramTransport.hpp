#pragma once
#include "FrameKit/Networking/Networking.hpp"

namespace FrameKit::Net {

	class UdpDatagramTransport final : public IDatagramTransport {
	public:
		UdpDatagramTransport();
		~UdpDatagramTransport() override;

		NetErr Open();
		NetErr SetBroadcast(bool enabled);
		NetErr SetNonBlocking(bool enabled);
		NetErr SetReuseAddr(bool enabled);

		// Dual-stack control:
		// false = accept IPv4-mapped addresses on the IPv6 socket
		NetErr SetIPv6Only(bool v6only);

		bool IsOpen() const override;

		NetErr Bind(const Endpoint& local) override;
		NetErr SendTo(const Endpoint& to, ConstByteSpan bytes) override;
		NetErr RecvFrom(ByteSpan out_buffer, RecvFromInfo& out) override;

		NetErr Close() override;

	private:
		void* impl_ = nullptr;
	};

} // namespace FrameKit::Net
