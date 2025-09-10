// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Application/Application.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      
// =============================================================================

#include "FrameKit/Application/Application.h"
#include "FrameKit/Core/Engine/LayerStack.h"


namespace FrameKit {
	bool Application::OnUpdate(Timestep ts) {
		// Default: update all layers
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
		if (m_LayerStack) {
			for (Layer* layer : *m_LayerStack) {
				if (layer) {
					layer->OnSyncUpdate(ts);
				}
			}
		}
		return true; // Return false to close the application
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