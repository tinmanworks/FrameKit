#pragma once
#include "FrameKit/Networking/Networking.hpp"

namespace FrameKit::Net {

    using MessageHandlerFn = void(*)(void* user, const HeaderView& hdr, ConstByteSpan payload, const Endpoint& from);

    struct Subscription {
        uint64_t topic = 0;
        uint16_t msg_type = 0;
        uint16_t _pad = 0;
        MessageHandlerFn fn = nullptr;
        void* user = nullptr;
    };

    struct DatagramChannelConfig {
        WireMode mode = WireMode::Framed;

        bool use_header_crc = false;
        bool use_payload_crc = false;

        uint32_t session = 0;
        bool enforce_session = false;

        uint32_t max_datagram = 1400;
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

        NetErr SendTo(const Endpoint& to, const HeaderView& hdr, ConstByteSpan payload);

        NetErr PollOnce();
        NetErr PollAll();

    private:
        void dispatch_(const HeaderView& hv, ConstByteSpan payload, const Endpoint& from);

        IDatagramTransport* transport_ = nullptr;
        DatagramChannelConfig cfg_{};
        HeaderCodecOps codec_{};

        Subscription subs_[64]{};
        uint32_t subs_count_ = 0;
    };

} // namespace FrameKit::Net
