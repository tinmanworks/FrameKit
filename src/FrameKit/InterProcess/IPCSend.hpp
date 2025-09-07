/*
 * Project: FrameKit
 * File: IPCSend.hpp
 * Author: George Gil
 * Date: 2025-08-22
 * License: MIT
 * Description:
 *   Helpers to send IPC events (with or without payload).
 */

#pragma once
#include <string>
#include "FrameKit/InterProcess/InterprocessEvent.hpp"
#include "FrameKit/InterProcess/InterprocessEventManager.h"

namespace FrameKit::IPC {

    inline void SendNoPayload(InterprocessEventManager& mgr, const char* name, uint32_t key) {
        InterprocessEvent e{ name };
        e.key = key; // GenerateKey(name) would match as well, but pass the canonical key
        mgr.PushDescriptor(e);
    }

    template <typename T>
    inline void SendWithPayload(InterprocessEventManager& mgr, const char* name, uint32_t key, const T& payload) {
        static_assert(std::is_trivially_copyable_v<T>, "Payload must be trivially copyable");
        InterprocessEvent e{ name };
        e.key = key;
        SetPayload(e, payload);
        mgr.PushDescriptor(e);
    }

} // namespace FrameKit::IPC
