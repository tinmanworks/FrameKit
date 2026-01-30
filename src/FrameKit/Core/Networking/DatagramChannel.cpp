#include "FrameKit/Networking/DatagramChannel.hpp"
#include <cstring>

// CRC32 symbol (defined in your existing CRC32.cpp)
namespace FrameKit::Net::Detail { uint32_t CRC32_IEEE(const uint8_t* data, size_t len); }

namespace FrameKit::Net {

    DatagramChannel::DatagramChannel() = default;
    DatagramChannel::~DatagramChannel() { (void)Close(); }

    NetErr DatagramChannel::Open(IDatagramTransport* transport,
        const DatagramChannelConfig& cfg,
        const HeaderCodecOps& codec) {
        if (!transport) return NetErr::InvalidArg;
        if (!codec.decode || !codec.encode) return NetErr::InvalidArg;
        if (cfg.max_subscriptions > 64) return NetErr::InvalidArg;

        transport_ = transport;
        cfg_ = cfg;
        codec_ = codec;
        subs_count_ = 0;
        return NetErr::Ok;
    }

    NetErr DatagramChannel::Close() {
        transport_ = nullptr;
        subs_count_ = 0;
        return NetErr::Ok;
    }

    bool DatagramChannel::IsOpen() const {
        return transport_ && transport_->IsOpen();
    }

    NetErr DatagramChannel::Subscribe(uint64_t topic, uint16_t msg_type, MessageHandlerFn fn, void* user) {
        if (!fn) return NetErr::InvalidArg;
        if (subs_count_ >= cfg_.max_subscriptions) return NetErr::NoCapacity;

        subs_[subs_count_] = Subscription{ topic, msg_type, 0, fn, user };
        ++subs_count_;
        return NetErr::Ok;
    }

    void DatagramChannel::dispatch_(const HeaderView& hv, ConstByteSpan payload, const Endpoint& from) {
        for (uint32_t i = 0; i < subs_count_; ++i) {
            const auto& s = subs_[i];
            if (s.topic == hv.topic && s.msg_type == hv.msg_type) {
                s.fn(s.user, hv, payload, from);
            }
        }
    }

    NetErr DatagramChannel::SendTo(const Endpoint& to, const HeaderView& hdr, ConstByteSpan payload) {
        if (!transport_) return NetErr::NotOpen;

        uint8_t buffer[2048];
        if (cfg_.max_datagram > sizeof(buffer)) return NetErr::TooLarge;

        if (cfg_.mode == WireMode::Raw) {
            const size_t header_len = codec_.encode(codec_.user, ByteSpan{ buffer, sizeof(buffer) }, hdr);
            if (header_len == 0) return NetErr::Fail;
            if (header_len + payload.size > cfg_.max_datagram) return NetErr::TooLarge;

            if (payload.size) std::memcpy(buffer + header_len, payload.data, payload.size);
            return transport_->SendTo(to, ConstByteSpan{ buffer, header_len + payload.size });
        }

        if (sizeof(PreHeader) >= cfg_.max_datagram) return NetErr::TooLarge;

        PreHeader ph{};
        ph.magic = FK_MAGIC_FKN1;
        ph.version = FK_PREHEADER_VERSION;
        ph.flags = 0;
        ph.session = cfg_.session;
        ph.header_crc = 0;
        ph.payload_crc = 0;

        const size_t off = sizeof(PreHeader);
        const size_t header_len = codec_.encode(codec_.user, ByteSpan{ buffer + off, sizeof(buffer) - off }, hdr);
        if (header_len == 0) return NetErr::Fail;

        if (off + header_len + payload.size > cfg_.max_datagram) return NetErr::TooLarge;

        ph.header_len = (uint16_t)header_len;
        ph.payload_len = (uint32_t)payload.size;

        if (cfg_.use_header_crc) {
            ph.flags |= (1u << 1);
            ph.header_crc = Detail::CRC32_IEEE(buffer + off, header_len);
        }
        if (cfg_.use_payload_crc) {
            ph.flags |= (1u << 2);
            ph.payload_crc = Detail::CRC32_IEEE(payload.data, payload.size);
        }

        std::memcpy(buffer, &ph, sizeof(ph));
        if (payload.size) std::memcpy(buffer + off + header_len, payload.data, payload.size);

        return transport_->SendTo(to, ConstByteSpan{ buffer, off + header_len + payload.size });
    }

    NetErr DatagramChannel::PollOnce() {
        if (!transport_) return NetErr::NotOpen;

        uint8_t buffer[2048];
        RecvFromInfo rf{};

        const auto e = transport_->RecvFrom(ByteSpan{ buffer, sizeof(buffer) }, rf);
        if (e != NetErr::Ok) return e;

        ConstByteSpan bytes{ buffer, rf.bytes };

        if (cfg_.mode == WireMode::Raw) {
            HeaderView hv{};
            size_t header_size = 0;
            if (!codec_.decode(codec_.user, bytes, hv, header_size)) return NetErr::Ok;
            if (header_size > bytes.size) return NetErr::Ok;

            dispatch_(hv, ConstByteSpan{ bytes.data + header_size, bytes.size - header_size }, rf.from);
            return NetErr::Ok;
        }

        if (bytes.size < sizeof(PreHeader)) return NetErr::Ok;

        PreHeader ph{};
        std::memcpy(&ph, bytes.data, sizeof(ph));
        if (ph.magic != FK_MAGIC_FKN1) return NetErr::Ok;
        if (ph.version != FK_PREHEADER_VERSION) return NetErr::Ok;

        const size_t need = sizeof(PreHeader) + (size_t)ph.header_len + (size_t)ph.payload_len;
        if (need > bytes.size) return NetErr::Ok;

        if (cfg_.enforce_session && cfg_.session != 0 && ph.session != cfg_.session) return NetErr::Ok;

        const uint8_t* app_hdr = bytes.data + sizeof(PreHeader);
        const uint8_t* payload = app_hdr + ph.header_len;

        if (cfg_.use_header_crc && (ph.flags & (1u << 1))) {
            if (Detail::CRC32_IEEE(app_hdr, ph.header_len) != ph.header_crc) return NetErr::Ok;
        }
        if (cfg_.use_payload_crc && (ph.flags & (1u << 2))) {
            if (Detail::CRC32_IEEE(payload, ph.payload_len) != ph.payload_crc) return NetErr::Ok;
        }

        HeaderView hv{};
        size_t header_size = 0;
        if (!codec_.decode(codec_.user, ConstByteSpan{ app_hdr, ph.header_len }, hv, header_size)) return NetErr::Ok;
        if (header_size != ph.header_len) return NetErr::Ok;

        dispatch_(hv, ConstByteSpan{ payload, ph.payload_len }, rf.from);
        return NetErr::Ok;
    }

    NetErr DatagramChannel::PollAll() {
        for (;;) {
            const auto e = PollOnce();
            if (e == NetErr::WouldBlock) return NetErr::Ok;
            if (e != NetErr::Ok) return e;
        }
    }

} // namespace FrameKit::Net
