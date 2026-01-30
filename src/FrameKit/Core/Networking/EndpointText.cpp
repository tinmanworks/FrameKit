#include "FrameKit/Networking/EndpointText.hpp"
#include <cctype>
#include <cstring>
#include <cstdio>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

namespace FrameKit::Net {

    static const char* skip_ws(const char* s) {
        while (s && *s && std::isspace((unsigned char)*s)) ++s;
        return s;
    }

    static bool parse_u16(const char* s, uint16_t& out) {
        if (!s || !*s) return false;
        unsigned v = 0;
        const char* p = s;
        while (*p) {
            if (*p < '0' || *p > '9') return false;
            v = v * 10u + (unsigned)(*p - '0');
            if (v > 65535u) return false;
            ++p;
        }
        out = (uint16_t)v;
        return true;
    }

    // Accepts:
    //  - "a.b.c.d"
    //  - "a.b.c.d:port"
    //  - "[v6]"
    //  - "[v6]:port"
    //  - "[fe80::1%3]:port"   (numeric scope id)
    // port_out_default used if port not specified
    NetErr ParseEndpoint(const char* text, Endpoint& out, uint16_t port_out_default)
    {
        if (!text) return NetErr::InvalidArg;
        text = skip_ws(text);
        if (!*text) return NetErr::InvalidArg;

        // IPv6 in brackets
        if (*text == '[') {
            const char* close = std::strchr(text, ']');
            if (!close) return NetErr::ParseError;

            // Copy inside brackets into temp buffer
            char addrbuf[128]{};
            const size_t len = (size_t)(close - (text + 1));
            if (len == 0 || len >= sizeof(addrbuf)) return NetErr::BufferTooSmall;
            std::memcpy(addrbuf, text + 1, len);
            addrbuf[len] = '\0';

            // Optional scope id: fe80::1%3
            uint32_t scope = 0;
            char* percent = std::strchr(addrbuf, '%');
            if (percent) {
                *percent = '\0';
                const char* scope_str = percent + 1;
                if (!*scope_str) return NetErr::ParseError;
                // numeric only
                unsigned sc = 0;
                for (const char* p = scope_str; *p; ++p) {
                    if (*p < '0' || *p > '9') return NetErr::ParseError;
                    sc = sc * 10u + (unsigned)(*p - '0');
                }
                scope = (uint32_t)sc;
            }

            uint8_t a16[16]{};
#if defined(_WIN32)
            IN6_ADDR in6{};
            if (InetPtonA(AF_INET6, addrbuf, &in6) != 1) return NetErr::ParseError;
            std::memcpy(a16, &in6, 16);
#else
            if (::inet_pton(AF_INET6, addrbuf, a16) != 1) return NetErr::ParseError;
#endif

            uint16_t port = port_out_default;

            // parse optional :port after ]
            const char* after = close + 1;
            if (*after == ':') {
                ++after;
                char portbuf[16]{};
                size_t plen = 0;
                while (after[plen] && !std::isspace((unsigned char)after[plen])) {
                    if (plen + 1 >= sizeof(portbuf)) return NetErr::BufferTooSmall;
                    portbuf[plen] = after[plen];
                    ++plen;
                }
                portbuf[plen] = '\0';
                if (!parse_u16(portbuf, port)) return NetErr::ParseError;
            }

            out = Endpoint::FromV6(a16, port, scope);
            return NetErr::Ok;
        }

        // Otherwise, try IPv4 with optional :port.
        // Split last ':' as port separator ONLY if the string contains no other ':'.
        // (since non-bracket IPv6 not supported here)
        const char* colon = std::strrchr(text, ':');
        const char* first_colon = std::strchr(text, ':');

        uint16_t port = port_out_default;
        char ipbuf[64]{};

        if (colon && colon == first_colon) {
            // single ':' -> assume IPv4:port
            const size_t iplen = (size_t)(colon - text);
            if (iplen == 0 || iplen >= sizeof(ipbuf)) return NetErr::BufferTooSmall;
            std::memcpy(ipbuf, text, iplen);
            ipbuf[iplen] = '\0';

            const char* portstr = colon + 1;
            char portbuf[16]{};
            size_t plen = 0;
            while (portstr[plen] && !std::isspace((unsigned char)portstr[plen])) {
                if (plen + 1 >= sizeof(portbuf)) return NetErr::BufferTooSmall;
                portbuf[plen] = portstr[plen];
                ++plen;
            }
            portbuf[plen] = '\0';
            if (!parse_u16(portbuf, port)) return NetErr::ParseError;
        }
        else {
            // no ':' -> IPv4 without port
            const size_t iplen = std::strlen(text);
            if (iplen == 0 || iplen >= sizeof(ipbuf)) return NetErr::BufferTooSmall;
            std::memcpy(ipbuf, text, iplen + 1);
        }

#if defined(_WIN32)
        IN_ADDR in4{};
        if (InetPtonA(AF_INET, ipbuf, &in4) != 1) return NetErr::ParseError;
        const uint32_t v4_net = (uint32_t)in4.S_un.S_addr;
        const uint32_t v4_host = ntohl(v4_net);
#else
        in_addr in4{};
        if (::inet_pton(AF_INET, ipbuf, &in4) != 1) return NetErr::ParseError;
        const uint32_t v4_host = ntohl((uint32_t)in4.s_addr);
#endif

        out = Endpoint::FromV4(v4_host, port);
        return NetErr::Ok;
    }

    NetErr FormatEndpoint(const Endpoint& ep, char* out, size_t out_cap)
    {
        if (!out || out_cap == 0) return NetErr::InvalidArg;
        out[0] = '\0';

        if (ep.family == AddressFamily::IPv4) {
            const uint32_t v4_net = htonl(ep.v4.addr_host);
#if defined(_WIN32)
            IN_ADDR a{};
            a.S_un.S_addr = v4_net;
            char ip[64]{};
            if (!InetNtopA(AF_INET, &a, ip, (DWORD)sizeof(ip))) return NetErr::Fail;
#else
            in_addr a{};
            a.s_addr = v4_net;
            char ip[64]{};
            if (!::inet_ntop(AF_INET, &a, ip, sizeof(ip))) return NetErr::Fail;
#endif
            const int n = std::snprintf(out, out_cap, "%s:%u", ip, (unsigned)ep.v4.port_host);
            return (n > 0 && (size_t)n < out_cap) ? NetErr::Ok : NetErr::BufferTooSmall;
        }

        // IPv6
#if defined(_WIN32)
        IN6_ADDR a{};
        std::memcpy(&a, ep.v6.addr, 16);
        char ip[128]{};
        if (!InetNtopA(AF_INET6, &a, ip, (DWORD)sizeof(ip))) return NetErr::Fail;
#else
        char ip[128]{};
        if (!::inet_ntop(AF_INET6, ep.v6.addr, ip, sizeof(ip))) return NetErr::Fail;
#endif

        if (ep.v6.scope_id != 0) {
            const int n = std::snprintf(out, out_cap, "[%s%%%u]:%u",
                ip, (unsigned)ep.v6.scope_id, (unsigned)ep.v6.port_host);
            return (n > 0 && (size_t)n < out_cap) ? NetErr::Ok : NetErr::BufferTooSmall;
        }
        else {
            const int n = std::snprintf(out, out_cap, "[%s]:%u", ip, (unsigned)ep.v6.port_host);
            return (n > 0 && (size_t)n < out_cap) ? NetErr::Ok : NetErr::BufferTooSmall;
        }
    }

} // namespace FrameKit::Net
