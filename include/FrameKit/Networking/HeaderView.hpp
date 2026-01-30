#pragma once
#include <cstdint>

namespace FrameKit::Net {

// Semantic routing contract FrameKit uses (wire layout is application-defined).
struct HeaderView {
  uint16_t msg_type = 0;
  uint16_t flags    = 0;
  uint32_t seq      = 0;

  uint64_t ts_us = 0;
  uint64_t topic = 0;
  uint64_t src   = 0;
  uint64_t dst   = 0;

  const void* extra_ptr = nullptr; // optional extension
  uint32_t    extra_size = 0;
};

} // namespace FrameKit::Net
