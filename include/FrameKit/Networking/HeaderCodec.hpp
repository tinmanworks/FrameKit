#pragma once
#include "Span.hpp"
#include "HeaderView.hpp"
#include <cstddef>

namespace FrameKit::Net {

// Application-supplied codec (no templates at ABI boundary).
struct HeaderCodecOps {
  void* user = nullptr;

  // bytes = application header bytes (not including preheader)
  // out_header_size must be set to bytes consumed (normally == bytes.size for framed mode)
  bool (*decode)(void* user, ConstByteSpan bytes, HeaderView& out_view, size_t& out_header_size) = nullptr;

  // returns bytes written to out buffer (application header)
  size_t (*encode)(void* user, ByteSpan out_bytes, const HeaderView& in_view) = nullptr;
};

} // namespace FrameKit::Net
