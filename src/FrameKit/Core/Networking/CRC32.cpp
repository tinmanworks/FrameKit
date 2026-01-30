#include <cstddef>
#include <cstdint>

namespace FrameKit::Net::Detail {
    uint32_t CRC32_IEEE(const uint8_t* data, size_t len) {
        uint32_t crc = 0xFFFFFFFFu;
        for (size_t i = 0; i < len; ++i) {
            crc ^= data[i];
            for (int b = 0; b < 8; ++b) {
                const uint32_t mask = (crc & 1u) ? 0xFFFFFFFFu : 0u;
                crc = (crc >> 1) ^ (0xEDB88320u & mask);
            }
        }
        return ~crc;
    }
}
