#pragma once
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>
#include <chrono>
#include <bit>   // std::endian

namespace FrameKit::Net {

    inline uint64_t NowUs() {
        using namespace std::chrono;
        return (uint64_t)duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
    }

    // ---- endian helpers (no winsock dependency) ----
    constexpr inline uint16_t BSwap16(uint16_t x) {
        return (uint16_t)((x << 8) | (x >> 8));
    }

    constexpr inline uint32_t BSwap32(uint32_t x) {
        return ((x & 0x000000FFu) << 24) |
            ((x & 0x0000FF00u) << 8) |
            ((x & 0x00FF0000u) >> 8) |
            ((x & 0xFF000000u) >> 24);
    }

    constexpr inline uint64_t BSwap64(uint64_t x) {
        return ((x & 0x00000000000000FFull) << 56) |
            ((x & 0x000000000000FF00ull) << 40) |
            ((x & 0x0000000000FF0000ull) << 24) |
            ((x & 0x00000000FF000000ull) << 8) |
            ((x & 0x000000FF00000000ull) >> 8) |
            ((x & 0x0000FF0000000000ull) >> 24) |
            ((x & 0x00FF000000000000ull) >> 40) |
            ((x & 0xFF00000000000000ull) >> 56);
    }

    constexpr inline uint32_t Hton32(uint32_t x) {
        if constexpr (std::endian::native == std::endian::little) return BSwap32(x);
        else return x;
    }
    constexpr inline uint32_t Ntoh32(uint32_t x) {
        if constexpr (std::endian::native == std::endian::little) return BSwap32(x);
        else return x;
    }

    constexpr inline uint64_t Hton64(uint64_t x) {
        if constexpr (std::endian::native == std::endian::little) return BSwap64(x);
        else return x;
    }
    constexpr inline uint64_t Ntoh64(uint64_t x) {
        if constexpr (std::endian::native == std::endian::little) return BSwap64(x);
        else return x;
    }

    // ---- message header ----
#pragma pack(push, 1)
    struct MsgHeader {
        uint8_t  type;
        uint32_t seq_net;
        uint64_t ts_us_net;
    };
#pragma pack(pop)

    struct DecodedPacket {
        uint8_t type = 0;
        uint32_t seq = 0;
        uint64_t ts_us = 0;
        const uint8_t* payload = nullptr;
        size_t payload_len = 0;
    };

    // Encode header + payload into a byte vector
    inline std::vector<uint8_t> Encode(uint8_t type, uint32_t seq, std::span<const uint8_t> payload) {
        MsgHeader h{};
        h.type = type;
        h.seq_net = Hton32(seq);
        h.ts_us_net = Hton64(NowUs());

        std::vector<uint8_t> out(sizeof(MsgHeader) + payload.size());
        std::memcpy(out.data(), &h, sizeof(h));
        if (!payload.empty())
            std::memcpy(out.data() + sizeof(h), payload.data(), payload.size());
        return out;
    }

    // Decode without copying (views into provided buffer)
    inline bool Decode(std::span<const uint8_t> bytes, DecodedPacket& out) {
        if (bytes.size() < sizeof(MsgHeader)) return false;
        MsgHeader h{};
        std::memcpy(&h, bytes.data(), sizeof(h));

        out.type = h.type;
        out.seq = Ntoh32(h.seq_net);
        out.ts_us = Ntoh64(h.ts_us_net);
        out.payload = bytes.data() + sizeof(MsgHeader);
        out.payload_len = bytes.size() - sizeof(MsgHeader);
        return true;
    }

} // namespace FrameKit::Net
