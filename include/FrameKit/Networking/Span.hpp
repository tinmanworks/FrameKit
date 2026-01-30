#pragma once
#include <cstddef>
#include <cstdint>

namespace FrameKit::Net {

template <class T>
struct Span {
  T* data = nullptr;
  size_t size = 0;

  constexpr bool empty() const { return size == 0; }
  constexpr T* begin() const { return data; }
  constexpr T* end() const { return data + size; }
};

using ByteSpan = Span<uint8_t>;
using ConstByteSpan = Span<const uint8_t>;

} // namespace FrameKit::Net
