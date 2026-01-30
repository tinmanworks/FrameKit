#pragma once
#include <cstdint>

namespace FrameKit::Net {

#pragma pack(push, 1)
struct PreHeader {
  uint32_t magic;       // "FKN1" little-endian
  uint8_t  version;     // preheader version
  uint8_t  flags;       // bit1: header_crc, bit2: payload_crc
  uint16_t header_len;  // app header length (bytes)
  uint32_t payload_len; // payload length (bytes)
  uint32_t session;     // 0 if unused
  uint32_t header_crc;  // optional
  uint32_t payload_crc; // optional
};
#pragma pack(pop)

static constexpr uint32_t FK_MAGIC_FKN1 = 0x314E4B46u; // "FKN1"
static constexpr uint8_t  FK_PREHEADER_VERSION = 1;

} // namespace FrameKit::Net
