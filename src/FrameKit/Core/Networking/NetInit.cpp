#include "FrameKit/Networking/NetInit.h"

#if defined(_WIN32)
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
#endif

namespace FrameKit::Net {

NetInit::NetInit() {
    if (s_inited) return;

#if defined(_WIN32)
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        throw std::runtime_error("FrameKit::NetInit: WSAStartup failed");
    }
#endif
    s_inited = true;
}

NetInit::~NetInit() {
#if defined(_WIN32)
    if (s_inited) {
        WSACleanup();
    }
#endif
    s_inited = false;
}

bool NetInit::IsInitialized() { return s_inited; }

} // namespace FrameKit::Net
