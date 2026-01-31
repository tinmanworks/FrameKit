#include "FrameKit/Networking/DatagramChannel.hpp"
#include "FrameKit/Debug/Log.h"

#include <cstring>

// CRC32 symbol (defined in your existing CRC32.cpp)
namespace FrameKit::Net::Detail { uint32_t CRC32_IEEE(const uint8_t* data, size_t len); }

namespace FrameKit::Net {

    static const char* fam_str(AddressFamily f) {
        return (f == AddressFamily::IPv4) ? "v4" : "v6";
    }

    static uint16_t ep_port(const Endpoint& e) {
        return (e.family == AddressFamily::IPv4) ? e.v4.port_host : e.v6.port_host;
    }

    // ----------------------------------------------------------------------------

    DatagramChannel::DatagramChannel() = default;
    DatagramChannel::~DatagramChannel() { (void)Close(); }

    NetErr DatagramChannel::Open(IDatagramTransport* transport,
        const DatagramChannelConfig& cfg,
        const HeaderCodecOps& codec)
    {
        if (!transport) return NetErr::InvalidArg;
        if (!codec.decode || !codec.encode) return NetErr::InvalidArg;
        if (cfg.max_subscriptions > 64) return NetErr::InvalidArg;

        transport_ = transport;
        cfg_ = cfg;
        codec_ = codec;
        subs_count_ = 0;

        FK_INFO("[Net] DatagramChannel::Open mode={} hdr_crc={} pay_crc={} session={} enforce={} max_dgram={} max_subs={}",
            (cfg_.mode == WireMode::Raw ? "raw" : "framed"),
            cfg_.use_header_crc ? 1 : 0,
            cfg_.use_payload_crc ? 1 : 0,
            (unsigned)cfg_.session,
            cfg_.enforce_session ? 1 : 0,
            (unsigned)cfg_.max_datagram,
            (unsigned)cfg_.max_subscriptions);

        return NetErr::Ok;
    }

    NetErr DatagramChannel::Close() {
        FK_INFO("[Net] DatagramChannel::Close subs={}", (unsigned)subs_count_);
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

        FK_INFO("[Net] Subscribe ok topic={} msg={} subs_now={}", (unsigned long long)topic, (unsigned)msg_type, (unsigned)subs_count_);
        return NetErr::Ok;
    }

    void DatagramChannel::dispatch_(const HeaderView& hv, ConstByteSpan payload, const Endpoint& from) {
        bool delivered = false;

        FK_TRACE("[Net][Dispatch] from={} port={} topic={} msg={} payload={}B subs={}",
            fam_str(from.family),
            (unsigned)ep_port(from),
            (unsigned long long)hv.topic,
            (unsigned)hv.msg_type,
            (unsigned)payload.size,
            (unsigned)subs_count_);

        for (uint32_t i = 0; i < subs_count_; ++i) {
            const auto& s = subs_[i];
            if (s.topic == hv.topic && s.msg_type == hv.msg_type) {
                delivered = true;
                FK_TRACE("[Net][Dispatch] DELIVER topic={} msg={} (sub#{})", (unsigned long long)hv.topic, (unsigned)hv.msg_type, (unsigned)i);
                s.fn(s.user, hv, payload, from);
            }
        }

        if (!delivered) {
            FK_WARN("[Net][Dispatch] DROP no-subscriber topic={} msg={}", (unsigned long long)hv.topic, (unsigned)hv.msg_type);
        }
    }

    NetErr DatagramChannel::SendTo(const Endpoint& to, const HeaderView& hdr, ConstByteSpan payload) {
        if (!transport_) return NetErr::NotOpen;

        uint8_t buffer[2048];
        if (cfg_.max_datagram > sizeof(buffer)) return NetErr::TooLarge;

        FK_TRACE("[Net][Send] to={} port={} mode={} topic={} msg={} seq={} payload={}B",
            fam_str(to.family),
            (unsigned)ep_port(to),
            (cfg_.mode == WireMode::Raw ? "raw" : "framed"),
            (unsigned long long)hdr.topic,
            (unsigned)hdr.msg_type,
            (unsigned)hdr.seq,
            (unsigned)payload.size);

        if (cfg_.mode == WireMode::Raw) {
            const size_t header_len = codec_.encode(codec_.user, ByteSpan{ buffer, sizeof(buffer) }, hdr);
            if (header_len == 0) { FK_ERROR("[Net][Send] raw encode failed"); return NetErr::Fail; }
            if (header_len + payload.size > cfg_.max_datagram) { FK_WARN("[Net][Send] too large raw total={} max={}", (unsigned)(header_len + payload.size), (unsigned)cfg_.max_datagram); return NetErr::TooLarge; }

            if (payload.size) std::memcpy(buffer + header_len, payload.data, payload.size);

            const auto e = transport_->SendTo(to, ConstByteSpan{ buffer, header_len + payload.size });
            if (e != NetErr::Ok) FK_ERROR("[Net][Send] transport failed err={}", (int)e);
            return e;
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
        if (header_len == 0) { FK_ERROR("[Net][Send] framed encode failed"); return NetErr::Fail; }

        if (off + header_len + payload.size > cfg_.max_datagram) {
            FK_WARN("[Net][Send] too large framed total={} max={}", (unsigned)(off + header_len + payload.size), (unsigned)cfg_.max_datagram);
            return NetErr::TooLarge;
        }

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

        const auto e = transport_->SendTo(to, ConstByteSpan{ buffer, off + header_len + payload.size });
        if (e != NetErr::Ok) FK_ERROR("[Net][Send] transport failed err={}", (int)e);
        return e;
    }

    NetErr DatagramChannel::PollOnce() {
        if (!transport_) return NetErr::NotOpen;

        uint8_t buffer[2048];
        RecvFromInfo rf{};

        const auto e = transport_->RecvFrom(ByteSpan{ buffer, sizeof(buffer) }, rf);
        if (e == NetErr::WouldBlock) {
            FK_TRACE("[Net][Recv] would-block");
            return e;
        }
        if (e != NetErr::Ok) {
            FK_ERROR("[Net][Recv] transport err={}", (int)e);
            return e;
        }

        FK_TRACE("[Net][Recv] bytes={} from={} port={}", (unsigned)rf.bytes, fam_str(rf.from.family), (unsigned)ep_port(rf.from));

        ConstByteSpan bytes{ buffer, rf.bytes };

        if (cfg_.mode == WireMode::Raw) {
            HeaderView hv{};
            size_t header_size = 0;
            if (!codec_.decode(codec_.user, bytes, hv, header_size)) {
                FK_WARN("[Net][Decode] raw decode failed bytes={}", (unsigned)bytes.size);
                return NetErr::Ok;
            }
            if (header_size > bytes.size) {
                FK_WARN("[Net][Decode] raw header_size={} > bytes={}", (unsigned)header_size, (unsigned)bytes.size);
                return NetErr::Ok;
            }

            FK_TRACE("[Net][Decode] raw topic={} msg={} seq={} hdr={}B payload={}B",
                (unsigned long long)hv.topic, (unsigned)hv.msg_type, (unsigned)hv.seq,
                (unsigned)header_size, (unsigned)(bytes.size - header_size));

            dispatch_(hv, ConstByteSpan{ bytes.data + header_size, bytes.size - header_size }, rf.from);
            return NetErr::Ok;
        }

        if (bytes.size < sizeof(PreHeader)) {
            FK_WARN("[Net][Frame] drop: too small bytes={} < prehdr={}", (unsigned)bytes.size, (unsigned)sizeof(PreHeader));
            return NetErr::Ok;
        }

        PreHeader ph{};
        std::memcpy(&ph, bytes.data, sizeof(ph));

        if (ph.magic != FK_MAGIC_FKN1) {
            FK_WARN("[Net][Frame] drop: bad magic=0x{:08X}", (unsigned)ph.magic);
            return NetErr::Ok;
        }
        if (ph.version != FK_PREHEADER_VERSION) {
            FK_WARN("[Net][Frame] drop: bad version={} expected={}", (unsigned)ph.version, (unsigned)FK_PREHEADER_VERSION);
            return NetErr::Ok;
        }

        const size_t need = sizeof(PreHeader) + (size_t)ph.header_len + (size_t)ph.payload_len;
        if (need > bytes.size) {
            FK_WARN("[Net][Frame] drop: truncated need={} bytes={}", (unsigned)need, (unsigned)bytes.size);
            return NetErr::Ok;
        }

        if (cfg_.enforce_session && cfg_.session != 0 && ph.session != cfg_.session) {
            FK_WARN("[Net][Frame] drop: session mismatch got={} expected={}", (unsigned)ph.session, (unsigned)cfg_.session);
            return NetErr::Ok;
        }

        const uint8_t* app_hdr = bytes.data + sizeof(PreHeader);
        const uint8_t* payload = app_hdr + ph.header_len;

        if (cfg_.use_header_crc && (ph.flags & (1u << 1))) {
            const uint32_t got = Detail::CRC32_IEEE(app_hdr, ph.header_len);
            if (got != ph.header_crc) {
                FK_WARN("[Net][CRC] drop: header crc mismatch got=0x{:08X} expected=0x{:08X}", (unsigned)got, (unsigned)ph.header_crc);
                return NetErr::Ok;
            }
        }
        if (cfg_.use_payload_crc && (ph.flags & (1u << 2))) {
            const uint32_t got = Detail::CRC32_IEEE(payload, ph.payload_len);
            if (got != ph.payload_crc) {
                FK_WARN("[Net][CRC] drop: payload crc mismatch got=0x{:08X} expected=0x{:08X}", (unsigned)got, (unsigned)ph.payload_crc);
                return NetErr::Ok;
            }
        }

        HeaderView hv{};
        size_t header_size = 0;
        if (!codec_.decode(codec_.user, ConstByteSpan{ app_hdr, ph.header_len }, hv, header_size)) {
            FK_WARN("[Net][Decode] framed decode failed hdr_len={}", (unsigned)ph.header_len);
            return NetErr::Ok;
        }
        if (header_size != ph.header_len) {
            FK_WARN("[Net][Decode] framed header_size={} != prehdr.header_len={}", (unsigned)header_size, (unsigned)ph.header_len);
            return NetErr::Ok;
        }

        FK_TRACE("[Net][Decode] framed topic={} msg={} seq={} hdr={}B payload={}B",
            (unsigned long long)hv.topic, (unsigned)hv.msg_type, (unsigned)hv.seq,
            (unsigned)ph.header_len, (unsigned)ph.payload_len);

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
