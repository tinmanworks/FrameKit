/*
 * Project: FrameKit
 * File: LayerStack.cpp
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   Implementation of LayerStack: manages owned Layer* and overlay ordering.
 */

#include "FrameKit/Core/LayerStack.h"

#include <algorithm> // std::find

namespace FrameKit
{
    LayerStack::~LayerStack() {
        for (Layer* layer : m_Layers) {
            if (layer) {
                layer->OnDetach();
                delete layer;
            }
        }
        m_Layers.clear();
        m_LayerInsertIndex = 0;
    }

    void LayerStack::PushLayer(Layer* layer) {
        // Insert before overlays
        m_Layers.emplace(m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex), layer);
        ++m_LayerInsertIndex;
    }

    void LayerStack::PushOverlay(Layer* overlay) {
        // Overlays go to the end
        m_Layers.emplace_back(overlay);
    }

    void LayerStack::PopLayer(Layer* layer) {
        // Search only within the "layers" partition
        const auto layersEnd = m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex);
        auto it = std::find(m_Layers.begin(), layersEnd, layer);
        if (it != layersEnd) {
            if (*it) {
                (*it)->OnDetach();
                delete* it;
            }
            m_Layers.erase(it);
            // One fewer layer before the overlay partition
            if (m_LayerInsertIndex > 0) {
                --m_LayerInsertIndex;
            }
        }
    }

    void LayerStack::PopOverlay(Layer* overlay) {
        // Search only within the "overlays" partition
        const auto overlaysBegin = m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex);
        auto it = std::find(overlaysBegin, m_Layers.end(), overlay);
        if (it != m_Layers.end()) {
            if (*it) {
                (*it)->OnDetach();
                delete* it;
            }
            m_Layers.erase(it);
        }
    }
} // namespace FrameKit
