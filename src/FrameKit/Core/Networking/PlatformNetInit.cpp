#include "PlatformNetInit.hpp"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

static int g_refs = 0;
static bool g_inited = false;

namespace FrameKit::Net::Detail {
    void PlatformNetAcquire() {
        if (!g_inited) {
            WSADATA wsa{};
            (void)WSAStartup(MAKEWORD(2, 2), &wsa);
            g_inited = true;
        }
        ++g_refs;
    }

    void PlatformNetRelease() {
        if (g_refs > 0) {
            --g_refs;
            if (g_refs == 0) WSACleanup();
        }
    }
}
#else
namespace FrameKit::Net::Detail {
    void PlatformNetAcquire() {}
    void PlatformNetRelease() {}
}
#endif
