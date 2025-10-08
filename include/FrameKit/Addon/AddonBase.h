// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Addon/AddonBase.h
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-09-28
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Legacy C++ addon base (kept for engine-internal use).
// =============================================================================

#pragma once

#include "FrameKit/Engine/Defines.h"
#include <cstdint>

namespace FrameKit {
    enum struct AddonVersionTag : std::uint8_t { Unknown = 0, V1 = 1 };

    struct AddonABIVersion { int major; int minor; int patch; };

    struct IAddonBase { virtual ~IAddonBase() = default; };
}

// Helper macro remains for legacy-only paths
#define FK_TAG_FROM_MAJ(M) FrameKit::AddonVersionTag::V##M

// Legacy export macro retained for compatibility if needed.
#define FK_DECLARE_ADDON(AddonClass, TAG, MAJOR, MINOR, PATCH) \
FK_API FrameKit::IAddonBase* FK_CDECL CreateAddonInstance() noexcept { \
    try { return new AddonClass(); } catch (...) { return nullptr; } \
} \
FK_API void FK_CDECL DestroyAddonInstance(FrameKit::IAddonBase* instance) noexcept { delete instance; } \
FK_API FrameKit::AddonVersionTag FK_CDECL GetAddonVersionTag() noexcept { return TAG; } \
FK_API void FK_CDECL GetAddonABIVersion(FrameKit::AddonABIVersion* version) noexcept { \
    *version = FrameKit::AddonABIVersion{ MAJOR, MINOR, PATCH }; \
}
