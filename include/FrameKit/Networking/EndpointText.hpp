#pragma once
#include "FrameKit/Networking/Networking.hpp"

namespace FrameKit::Net {

	// Parses:
	//   - "192.168.1.10:5000"
	//   - "[2001:db8::1]:5000"
	//   - "[fe80::1%3]:5000"   (scope id numeric)
	// Port is optional; if missing, port_out_default is used.
	NetErr ParseEndpoint(const char* text, Endpoint& out, uint16_t port_out_default = 0);

	// Writes:
	//   - IPv4: "192.168.1.10:5000"
	//   - IPv6: "[2001:db8::1]:5000"  (scope id printed as %<n> if nonzero)
	NetErr FormatEndpoint(const Endpoint& ep, char* out, size_t out_cap);

} // namespace FrameKit::Net
