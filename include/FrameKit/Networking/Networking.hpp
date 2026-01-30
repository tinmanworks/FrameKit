#pragma once
#include <cstdint>
#include <cstddef>

namespace FrameKit::Net {

    // --------------------
    // Span (tiny, no STL)
    // --------------------
    template <class T>
    struct Span {
        T* data = nullptr;
        size_t size = 0;
        constexpr bool empty() const { return size == 0; }
    };
    using ByteSpan = Span<uint8_t>;
    using ConstByteSpan = Span<const uint8_t>;

    // --------------------
    // Errors / enums
    // --------------------
    enum class NetErr : int32_t {
        Ok = 0,
        Fail = -1,
        WouldBlock = -2,
        InvalidArg = -3,
        NotOpen = -4,
        TooLarge = -5,
        NoCapacity = -6,
        ParseError = -7,
        BufferTooSmall = -8,
    };

    enum class SocketKind : uint8_t { Unknown = 0, Udp = 1, Tcp = 2 };
    enum class WireMode : uint8_t { Raw = 0, Framed = 1 };

    // --------------------
    // Endpoints
    // --------------------
    struct IPv4Endpoint {
        uint32_t addr_host = 0; // host order
        uint16_t port_host = 0; // host order
    };

    struct IPv6Endpoint {
        uint8_t  addr[16]{};     // raw bytes
        uint32_t scope_id = 0;   // link-local scope id (0 if none)
        uint16_t port_host = 0;  // host order
    };

    enum class AddressFamily : uint8_t { IPv4 = 4, IPv6 = 6 };

    struct Endpoint {
        AddressFamily family = AddressFamily::IPv4;
        union {
            IPv4Endpoint v4;
            IPv6Endpoint v6;
        };

        Endpoint() : family(AddressFamily::IPv4), v4{} {}

        static Endpoint FromV4(uint32_t addr_host, uint16_t port_host) {
            Endpoint e;
            e.family = AddressFamily::IPv4;
            e.v4.addr_host = addr_host;
            e.v4.port_host = port_host;
            return e;
        }

        static Endpoint FromV6(const uint8_t addr16[16], uint16_t port_host, uint32_t scope_id = 0) {
            Endpoint e;
            e.family = AddressFamily::IPv6;
            for (int i = 0; i < 16; ++i) e.v6.addr[i] = addr16[i];
            e.v6.port_host = port_host;
            e.v6.scope_id = scope_id;
            return e;
        }
    };

    // Optional back-compat alias if you want it
    using EndPoint = Endpoint;

    // --------------------
    // Recv metadata (dual-stack)
    // NOTE: Renamed to avoid collision with method name RecvFrom(...)
    // --------------------
    struct RecvFromInfo {
        Endpoint from{};
        size_t bytes = 0;
    };

    // --------------------
    // FrameKit preheader
    // --------------------
#pragma pack(push, 1)
    struct PreHeader {
        uint32_t magic;       // "FKN1" little-endian
        uint8_t  version;     // preheader version
        uint8_t  flags;       // bit1: header_crc, bit2: payload_crc
        uint16_t header_len;  // app header length bytes
        uint32_t payload_len; // payload length bytes
        uint32_t session;     // 0 if unused
        uint32_t header_crc;  // optional
        uint32_t payload_crc; // optional
    };
#pragma pack(pop)

    static constexpr uint32_t FK_MAGIC_FKN1 = 0x314E4B46u; // "FKN1"
    static constexpr uint8_t  FK_PREHEADER_VERSION = 1;

    // --------------------
    // Header semantic view + codec ops
    // --------------------
    struct HeaderView {
        uint16_t msg_type = 0;
        uint16_t flags = 0;
        uint32_t seq = 0;
        uint64_t ts_us = 0;
        uint64_t topic = 0;
        uint64_t src = 0;
        uint64_t dst = 0;

        const void* extra_ptr = nullptr;
        uint32_t    extra_size = 0;
    };

    struct HeaderCodecOps {
        void* user = nullptr;

        bool (*decode)(void* user, ConstByteSpan bytes, HeaderView& out_view, size_t& out_header_size) = nullptr;
        size_t(*encode)(void* user, ByteSpan out_bytes, const HeaderView& in_view) = nullptr;
    };

    // --------------------
    // Socket base
    // --------------------
    class Socket {
    public:
        virtual ~Socket() = default;
        virtual SocketKind Kind() const = 0;
        virtual bool IsOpen() const = 0;
        virtual NetErr Close() = 0;
        virtual NetErr SetNonBlocking(bool enabled) = 0;
        virtual NetErr SetReuseAddr(bool enabled) = 0;
    };

    // --------------------
    // Datagram transport (dual-stack)
    // --------------------
    class IDatagramTransport {
    public:
        virtual ~IDatagramTransport() = default;
        virtual bool IsOpen() const = 0;

        virtual NetErr Bind(const Endpoint& local) = 0;
        virtual NetErr SendTo(const Endpoint& to, ConstByteSpan bytes) = 0;
        virtual NetErr RecvFrom(ByteSpan out_buffer, RecvFromInfo& out) = 0;

        virtual NetErr Close() = 0;
    };

} // namespace FrameKit::Net
