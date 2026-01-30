#pragma once
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>
#include <chrono>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>   // htonl/ntohl
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>  // htonl/ntohl
#endif

namespace FrameKit::Net {

    inline uint64_t NowUs() {
        using namespace std::chrono;
        return (uint64_t)duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
    }

    inline uint32_t Hton32(uint32_t x) { return htonl(x); }
    inline uint32_t Ntoh32(uint32_t x) { return ntohl(x); }

    // Portable 64-bit host/network conversion using htonl
    inline uint64_t Hton64(uint64_t x) {
        uint32_t hi = (uint32_t)(x >> 32);
        uint32_t lo = (uint32_t)(x & 0xFFFFFFFFULL);
        return (uint64_t)htonl(lo) << 32 | (uint64_t)htonl(hi);
    }
    inline uint64_t Ntoh64(uint64_t x) { return Hton64(x); }

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
