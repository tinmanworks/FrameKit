/*
 * Project: FrameKit
 * File: InterprocessEvent.hpp
 * Author: George Gil
 * Date: 2025-08-22
 * License: MIT
 * Description:
 *   Defines the InterprocessEvent structure and helper functions for
 *   payload serialization/deserialization.
 *   - Deterministic 31× rolling hash for routing keys.
 *   - Fixed-size payload buffer (10 KiB).
 *   - Name field for debugging/traceability.
 */

#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <algorithm> // for std::min

namespace FrameKit::IPC {

// Deterministic key generator (not std::hash, ensures cross-process stability).
inline uint32_t GenerateKey(const char* s) {
    uint32_t h = 0;
    while (*s) { h = h * 31u + static_cast<uint8_t>(*s++); }
    return h;
}

inline constexpr std::size_t kPayloadSize = 1024 * 10; // 10 KiB

struct InterprocessEvent {
    uint32_t key = 0;             // routing key (from GenerateKey)
    char     name[64]{};          // debug label (e.g., function name)
    uint8_t  payload[kPayloadSize]{}; // raw data payload

    InterprocessEvent() = default;

    explicit InterprocessEvent(const std::string& n) {
        key = GenerateKey(n.c_str());
        std::memset(name, 0, sizeof(name));
        std::memcpy(name, n.data(), std::min(n.size(), sizeof(name)));
    }
};

// Deserialize payload into trivially copyable type T.
template <typename T>
inline const T& GetPayload(const uint8_t* p) {
    static_assert(std::is_trivially_copyable_v<T>, "Payload type must be trivially copyable");
    return *reinterpret_cast<const T*>(p);
}

// Serialize trivially copyable type T into the event payload.
template <typename T>
inline void SetPayload(InterprocessEvent& e, const T& v) {
    static_assert(std::is_trivially_copyable_v<T>, "Payload type must be trivially copyable");
    static_assert(sizeof(T) <= kPayloadSize, "Payload too large for buffer");
    std::memcpy(e.payload, &v, sizeof(T));
}

} // namespace FrameKit::IPC
