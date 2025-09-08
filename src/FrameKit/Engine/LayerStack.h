// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Engine/LayerStack.h
// Author       : George Gil
// Created      : 2025-09-08
// Updated      : 2025-09-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//      Container for managing engine layers and overlays. Owns Layer* pointers:
//      calls OnDetach() and deletes layers on removal and destruction.
// =============================================================================

#pragma once

#include "FrameKit/Engine/Layer.h"

#include <vector>
#include <cstddef> // std::size_t

namespace FrameKit
{
    class LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack();

        // Non-copyable / non-movable: this class OWNS raw Layer*
        LayerStack(const LayerStack&) = delete;
        LayerStack& operator=(const LayerStack&) = delete;
        LayerStack(LayerStack&&) = delete;
        LayerStack& operator=(LayerStack&&) = delete;

        // Ownership: pushes transfer ownership of the pointer to LayerStack
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        // If found, calls OnDetach() and deletes the owned pointer
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);

        // Iteration (non-const)
        [[nodiscard]] std::vector<Layer*>::iterator begin() noexcept { return m_Layers.begin(); }
        [[nodiscard]] std::vector<Layer*>::iterator end()   noexcept { return m_Layers.end(); }
        [[nodiscard]] std::vector<Layer*>::reverse_iterator rbegin() noexcept { return m_Layers.rbegin(); }
        [[nodiscard]] std::vector<Layer*>::reverse_iterator rend()   noexcept { return m_Layers.rend(); }

        // Iteration (const)
        [[nodiscard]] std::vector<Layer*>::const_iterator begin() const noexcept { return m_Layers.begin(); }
        [[nodiscard]] std::vector<Layer*>::const_iterator end()   const noexcept { return m_Layers.end(); }
        [[nodiscard]] std::vector<Layer*>::const_reverse_iterator rbegin() const noexcept { return m_Layers.rbegin(); }
        [[nodiscard]] std::vector<Layer*>::const_reverse_iterator rend()   const noexcept { return m_Layers.rend(); }

    private:
        std::vector<Layer*> m_Layers;
        std::size_t         m_LayerInsertIndex = 0; // layers [0, m_LayerInsertIndex) / overlays [m_LayerInsertIndex, end)
    };
} // namespace FrameKit
