// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Interface/GlobalContext.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//         Backend-agnostic context handle and app-global context.
// =============================================================================

#pragma once

#include "FrameKit/Base/Base.h"

#include <string_view>

namespace FrameKit {

    // Opaque handle to any Interface Backend context (e.g. ImGuiContext).
    // 'tag'identifies the backend type; adapters provide a unique tag.
    struct InterfaceContextHandle {
        const void* tag = nullptr;
        void*       ptr = nullptr;

        constexpr InterfaceContextHandle() noexcept = default;
        constexpr InterfaceContextHandle(const void* t, void* p) noexcept : tag(t), ptr(p) {}

        FK_NODISCARD constexpr bool valid() const noexcept { return tag != nullptr && ptr != nullptr; }

        template<typename T>
        FK_NODISCARD T* as(const void* expected_tag) const noexcept {
            return (tag == expected_tag) ? static_cast<T*>(ptr) : nullptr;
        }
    };

    struct GlobalContext
    {
        InterfaceContextHandle interfaceCtx;
        void*                  mainWindow = nullptr;

        constexpr GlobalContext() noexcept = default;
        explicit constexpr GlobalContext(InterfaceContextHandle ctx) noexcept : interfaceCtx(ctx) {}

        FK_NODISCARD constexpr bool valid() const noexcept { return interfaceCtx.valid(); }
    };

} // namespace FrameKit