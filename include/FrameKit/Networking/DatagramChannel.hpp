#pragma once
#include "DatagramTransport.hpp"
#include "HeaderCodec.hpp"
#include "PreHeader.hpp"
#include "NetTypes.hpp"

namespace FrameKit::Net {

using MessageHandlerFn = void(*)(void* user, const HeaderView& hdr, ConstByteSpan payload, const IPv4Endpoint& from);

struct Subscription {
  uint64_t topic = 0;
  uint16_t msg_type = 0;
  uint16_t _pad = 0;
  MessageHandlerFn fn = nullptr;
  void* user = nullptr;
};

struct DatagramChannelConfig {
  WireMode mode = WireMode::Framed;

  bool use_header_crc  = false;
  bool use_payload_crc = false;

  uint32_t session = 0;        // tagged on outgoing framed packets
  bool enforce_session = false; // drop incoming framed packets with mismatched session when session != 0

  uint32_t max_datagram = 1400;

  // Capacity is fixed to 64 in this implementation (no heap in public API).
  // Keep this for future ABI evolution; currently must be <= 64.
  uint32_t max_subscriptions = 64;
};

class DatagramChannel {
public:
  DatagramChannel();
  ~DatagramChannel();

  NetErr Open(IDatagramTransport* transport,
              const DatagramChannelConfig& cfg,
              const HeaderCodecOps& codec);

  NetErr Close();
  bool IsOpen() const;

  NetErr Subscribe(uint64_t topic, uint16_t msg_type, MessageHandlerFn fn, void* user);

  NetErr SendTo(const IPv4Endpoint& to, const HeaderView& hdr, ConstByteSpan payload);

  // Non-blocking pumping. Call from your loop.
  NetErr PollOnce(); // processes max 1 datagram
  NetErr PollAll();  // drains

private:
  void dispatch_(const HeaderView& hv, ConstByteSpan payload, const IPv4Endpoint& from);

  IDatagramTransport* transport_ = nullptr;
  DatagramChannelConfig cfg_{};
  HeaderCodecOps codec_{};

  Subscription subs_[64]{};
  uint32_t subs_count_ = 0;
};

} // namespace FrameKit::Net
