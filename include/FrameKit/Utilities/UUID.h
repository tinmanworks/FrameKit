// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Utilities/UUID.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 64-bit UUID wrapper with random generation, comparisons,
//                and std::hash support for unordered containers.
// =============================================================================

#pragma once

#include <cstdint>

namespace FrameKit
{
    class UUID
    {
    public:
        // Constructs a new random UUID
        UUID();

        // Construct from a known 64-bit value
        explicit UUID(std::uint64_t uuid) noexcept
            : m_UUID(uuid) {
        }

        // Defaulted copy/move
        UUID(const UUID&) = default;
        UUID(UUID&&) = default;
        UUID& operator=(const UUID&) = default;
        UUID& operator=(UUID&&) = default;

        // Accessors
        [[nodiscard]] std::uint64_t value() const noexcept { return m_UUID; }

        // Implicit conversion kept for backward compatibility
        operator std::uint64_t() const noexcept { return m_UUID; }

        // Comparisons
        friend bool operator==(const UUID& a, const UUID& b) noexcept { return a.m_UUID == b.m_UUID; }
        friend bool operator!=(const UUID& a, const UUID& b) noexcept { return a.m_UUID != b.m_UUID; }
        friend bool operator<(const UUID& a, const UUID& b)  noexcept { return a.m_UUID < b.m_UUID; }

    private:
        std::uint64_t m_UUID = 0;
    };
} // namespace FrameKit

// Hash support for unordered containers
#include <cstddef> // std::size_t
#include <functional>

namespace std
{
    template<>
    struct hash<FrameKit::UUID>
    {
        std::size_t operator()(const FrameKit::UUID& uuid) const noexcept {
            return static_cast<std::size_t>(uuid.value());
        }
    };
}
