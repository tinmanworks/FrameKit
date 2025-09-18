// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Application/Application.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      
// =============================================================================

#include "FrameKit/Application/Application.h"
#include "FrameKit/Core/Engine/LayerStack.h"
#include "FrameKit/Debug/Log.h"

namespace FrameKit {
	bool Application::OnUpdate(Timestep ts) {
		// Default: update all layers
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);

		if (!m_LayerStack) {
			FK_CORE_WARN("OnUpdate skipped: LayerStack is null");
			return true;
		}

		size_t idx = 0;
		FK_CORE_TRACE("OnUpdate begin: layers={} dt={} ms", m_LayerStack->Size(), ts.Milliseconds());

		for (Layer* layer : *m_LayerStack) {
			if (!layer) {
				FK_CORE_WARN("OnUpdate: layer[{}] is null", idx++);
				continue;
			}
			layer->OnSyncUpdate(ts);
			FK_CORE_TRACE("OnUpdate: layer[{}] updated", idx++);
		}

		FK_CORE_TRACE("OnUpdate end");
		return true; // Return false to request shutdown
	}

	void Application::OnRender() {
		// Default: render all layers
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
		if (m_LayerStack) {
			for (Layer* layer : *m_LayerStack) {
				if (layer) {
					layer->OnRender();
				}
			}
		}
	}

	void Application::OnEvent(Event& e) {
		// Default: propagate to layers in reverse order
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
		if (m_LayerStack) {
			for (auto it = m_LayerStack->rbegin(); it != m_LayerStack->rend(); ++it) {
				Layer* layer = *it;
				if (layer) {
					layer->OnEvent(e);
					//if (e.Handled) break;
					break; // For now, do not propagate to multiple layers
				}
			}
		}
	}
}