// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Engine/Memory.h 
// Author       : George Gil
// Created      : 2025-09-09
// Updated      : 2025-09-09
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Smart pointer aliases and creation helpers.
// =============================================================================

#pragma once
#include <memory>
#include <utility>
#include "FrameKit/Engine/Defines.h"

namespace FrameKit {

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, typename... Args>
    FK_FORCE_INLINE Scope<T> CreateScope(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename... Args>
    FK_FORCE_INLINE Ref<T> CreateRef(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

} // namespace FrameKit
