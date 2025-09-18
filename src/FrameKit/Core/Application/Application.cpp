// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Application/Application.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Default Application overrides for update, render, and event dispatch.
//      Iterates through layers, calls hooks, and logs activity.
// =============================================================================

#include "FrameKit/Application/Application.h"
#include "FrameKit/Core/Engine/LayerStack.h"
#include "FrameKit/Debug/Log.h"

namespace FrameKit {
	bool Application::OnUpdate(Timestep ts) {
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
		return true;
	}

	void Application::OnRender() {
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);

		if (!m_LayerStack) {
			FK_CORE_WARN("OnRender skipped: LayerStack is null");
			return;
		}

		FK_CORE_TRACE("OnRender begin: layers={}", m_LayerStack->Size());

		size_t idx = 0;
		for (Layer* layer : *m_LayerStack) {
			if (!layer) {
				FK_CORE_WARN("OnRender: layer[{}] is null", idx++);
				continue;
			}
			layer->OnRender();
			FK_CORE_TRACE("OnRender: layer[{}] rendered", idx++);
		}

		FK_CORE_TRACE("OnRender end");
	}

	void Application::OnEvent(Event& e) {
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);

		if (!m_LayerStack) {
			FK_CORE_WARN("OnEvent skipped: LayerStack is null");
			return;
		}

		FK_CORE_TRACE("OnEvent dispatch begin: layers={}", m_LayerStack->Size());

		for (auto it = m_LayerStack->rbegin(); it != m_LayerStack->rend(); ++it) {
			Layer* layer = *it;
			if (!layer) {
				FK_CORE_WARN("OnEvent: null layer in reverse iteration");
				continue;
			}
			layer->OnEvent(e);
			FK_CORE_TRACE("OnEvent: delivered to one layer, stop propagation");
			break; // current behavior: single-target delivery
			// if (e.Handled) { FK_CORE_TRACE("OnEvent: handled, stop"); break; }
		}

		FK_CORE_TRACE("OnEvent dispatch end");
	}
} // namespace FrameKit